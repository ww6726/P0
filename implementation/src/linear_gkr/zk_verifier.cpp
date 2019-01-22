#include "linear_gkr/zk_verifier.h"
#include <string>
#include <utility>
#include <gmp.h>
#include <vector>
#include <gmpxx.h>
#include <iostream>
#include "linear_gkr/random_generator.h"

void zk_verifier::get_prover(zk_prover *pp)
{
	p = pp;
}

void zk_verifier::read_circuit(const char *path)
{
	int d;
	static char str[300];
	FILE *circuit_in;
	circuit_in = fopen(path, "r");

	fscanf(circuit_in, "%d", &d);
	int n;
	C.circuit = new layer[d];
	C.total_depth = d;
	int max_bit_length = -1;
	for(int i = 0; i < d; ++i)
	{
		fscanf(circuit_in, "%d", &n);
		if(n == 1)
			C.circuit[i].gates = new gate[2];
		else
			C.circuit[i].gates = new gate[n];
		int max_gate = -1;
		int previous_g = -1;
		for(int j = 0; j < n; ++j)
		{
			int ty, g, u, v;
			fscanf(circuit_in, "%d%d%d%d", &ty, &g, &u, &v);
			if(g != previous_g + 1)
			{
				printf("Error, gates must be in sorted order, and full [0, 2^n - 1].");
			}
			previous_g = g;
			C.circuit[i].gates[g] = gate(ty, u, v);
		}
		max_gate = previous_g;
		int cnt = 0;
		while(max_gate)
		{
			cnt++;
			max_gate >>= 1;
		}
		max_gate = 1;
		while(cnt)
		{
			max_gate <<= 1;
			cnt--;
		}
		int mx_gate = max_gate;
		while(mx_gate)
		{
			cnt++;
			mx_gate >>= 1;
		}
		if(n == 1)
		{
			//add a dummy gate to avoid ill-defined layer.
			C.circuit[i].gates[max_gate] = gate(2, 0, 0);
			C.circuit[i].bit_length = cnt;
		}
		else
		{
			C.circuit[i].bit_length = cnt - 1;
		}
		fprintf(stderr, "layer %d, bit_length %d\n", i, C.circuit[i].bit_length);
		if(C.circuit[i].bit_length > max_bit_length)
			max_bit_length = C.circuit[i].bit_length;
	}
	p -> init_array(max_bit_length);

	beta_g_r0 = new prime_field::field_element[(1 << max_bit_length)];
	beta_g_r1 = new prime_field::field_element[(1 << max_bit_length)];
	beta_v = new prime_field::field_element[(1 << max_bit_length)];
	beta_u = new prime_field::field_element[(1 << max_bit_length)];
	fclose(circuit_in);
}

prime_field::field_element zk_verifier::add(int depth)
{
	//brute force for sanity check
	//it's slow
	prime_field::field_element ret = prime_field::field_element(0);
	for(int i = 0; i < (1 << C.circuit[depth].bit_length); ++i)
	{
		int g = i, u = C.circuit[depth].gates[i].u, v = C.circuit[depth].gates[i].v;
		if(C.circuit[depth].gates[i].ty == 0)
		{
			ret = ret + (beta_g_r0[g] + beta_g_r1[g]) * beta_u[u] * beta_v[v];
		}
	}
	ret.value = ret.value % prime_field::mod;
	if(ret.value < 0)
		ret.value = ret.value + prime_field::mod;
	return ret;
}
prime_field::field_element zk_verifier::mult(int depth)
{
	prime_field::field_element ret = prime_field::field_element(0);
	for(int i = 0; i < (1 << C.circuit[depth].bit_length); ++i)
	{
		int g = i, u = C.circuit[depth].gates[i].u, v = C.circuit[depth].gates[i].v;
		if(C.circuit[depth].gates[i].ty == 1)
		{
			ret = ret + (beta_g_r0[g] + beta_g_r1[g]) * beta_u[u] * beta_v[v];
		}
	}
	ret.value = ret.value % prime_field::mod;
	if(ret.value < 0)
		ret.value = ret.value + prime_field::mod;
	return ret;
}

void zk_verifier::beta_init(int depth, prime_field::field_element alpha, prime_field::field_element beta,
	const prime_field::field_element* r_0, const prime_field::field_element* r_1, 
	const prime_field::field_element* r_u, const prime_field::field_element* r_v, 
	const prime_field::field_element* one_minus_r_0, const prime_field::field_element* one_minus_r_1, 
	const prime_field::field_element* one_minus_r_u, const prime_field::field_element* one_minus_r_v)
{
	beta_g_r0[0] = alpha;
	beta_g_r1[0] = beta;
	for(int i = 0; i < C.circuit[depth].bit_length; ++i)
	{
		for(int j = 0; j < (1 << i); ++j)
		{
			beta_g_r0[j | (1 << i)].value = beta_g_r0[j].value * r_0[i].value % prime_field::mod;
			beta_g_r1[j | (1 << i)].value = beta_g_r1[j].value * r_1[i].value % prime_field::mod;
		}
		for(int j = 0; j < (1 << i); ++j)
		{
			beta_g_r0[j].value = beta_g_r0[j].value * one_minus_r_0[i].value % prime_field::mod;
			beta_g_r1[j].value = beta_g_r1[j].value * one_minus_r_1[i].value % prime_field::mod;
		}
	}
	beta_u[0] = prime_field::field_element(1);
	beta_v[0] = prime_field::field_element(1);
	for(int i = 0; i < C.circuit[depth - 1].bit_length; ++i)
	{
		for(int j = 0; j < (1 << i); ++j)
		{
			beta_u[j | (1 << i)] = beta_u[j] * r_u[i];
			beta_v[j | (1 << i)] = beta_v[j] * r_v[i];
		}
			
		for(int j = 0; j < (1 << i); ++j)
		{
			beta_u[j] = beta_u[j] * one_minus_r_u[i];
			beta_v[j] = beta_v[j] * one_minus_r_v[i];
		}
	}
}

prime_field::field_element* generate_randomness(unsigned int size)
{
	int k = size;
	prime_field::field_element* ret;
	ret = new prime_field::field_element[k];

	for(int i = 0; i < k; ++i)
	{
		ret[i] = prime_field::random();
		ret[i].value = ret[i].value % prime_field::mod;
	}
	return ret;
}

prime_field::field_element zk_verifier::V_in(const prime_field::field_element* r_0, const prime_field::field_element* one_minus_r_0,
								prime_field::field_element* output_raw, int r_0_size, int output_size)
{
	prime_field::field_element* output = new prime_field::field_element[output_size];
	for(int i = 0; i < output_size; ++i)
		output[i] = output_raw[i];
	for(int i = 0; i < r_0_size; ++i)
	{
		int last_gate;
		int cnt = 0;
		for(int j = 0; j < (output_size >> 1); ++j)
			output[j] = output[j << 1] * (one_minus_r_0[i]) + output[j << 1 | 1] * (r_0[i]);
		output_size >>= 1;
	}
	auto ret = output[0];
	ret.value = ret.value % prime_field::mod;
	delete[] output;
	if(ret.value < 0)
		ret.value = ret.value + prime_field::mod;
	return ret;
}

bool zk_verifier::verify()
{
	prime_field::init_random();
	p -> proof_init();

	auto result = p -> evaluate();

	prime_field::field_element *input;
	input = new prime_field::field_element[(1 << C.circuit[0].bit_length)];

	for(int i = 0; i < (1 << C.circuit[0].bit_length); ++i)
	{
		int g = i;
		if(C.circuit[0].gates[g].ty == 3)
		{
			input[g] = prime_field::field_element(C.circuit[0].gates[g].u);
		}
		else
			assert(false);
	}

	auto digest_input = p -> keygen_and_commit(C.circuit[0].bit_length);

	prime_field::field_element alpha, beta;
	alpha.value = 1;
	beta.value = 0;
	random_oracle oracle;
	//initial random value
	prime_field::field_element *r_0 = generate_randomness(C.circuit[C.total_depth - 1].bit_length), *r_1 = generate_randomness(C.circuit[C.total_depth - 1].bit_length);
	prime_field::field_element *one_minus_r_0, *one_minus_r_1;
	one_minus_r_0 = new prime_field::field_element[C.circuit[C.total_depth - 1].bit_length];
	one_minus_r_1 = new prime_field::field_element[C.circuit[C.total_depth - 1].bit_length];

	for(int i = 0; i < (C.circuit[C.total_depth - 1].bit_length); ++i)
	{
		one_minus_r_0[i] = prime_field::field_element(1) - r_0[i];
		one_minus_r_1[i] = prime_field::field_element(1) - r_1[i];
	}
	
	std::chrono::high_resolution_clock::time_point t_a = std::chrono::high_resolution_clock::now();
	std::cerr << "Calc V_output(r)" << std::endl;
	prime_field::field_element a_0 = p -> V_res(one_minus_r_0, r_0, result, C.circuit[C.total_depth - 1].bit_length, (1 << (C.circuit[C.total_depth - 1].bit_length)));
	
	std::chrono::high_resolution_clock::time_point t_b = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> ts = std::chrono::duration_cast<std::chrono::duration<double>>(t_b - t_a);
	std::cerr << "	Time: " << ts.count() << std::endl;
	a_0 = alpha * a_0;
	prime_field::field_element a_1 = prime_field::field_element(0); //* beta

	prime_field::field_element alpha_beta_sum = a_0; //+ a_1

	for(int i = C.total_depth - 1; i >= 1; --i)
	{
		std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();
		std::cerr << "Bound u start" << std::endl;
		auto rho = prime_field::random();
		std::vector<bn::Ec1> digest_mask;

		auto digest_maskR = p -> sumcheck_init(i, C.circuit[i].bit_length, C.circuit[i - 1].bit_length, C.circuit[i - 1].bit_length, alpha, beta, r_0, r_1, one_minus_r_0, one_minus_r_1);
		digest_mask = p -> generate_maskpoly_pre_rho(C.circuit[i - 1].bit_length * 2 + 1, 2);
		p -> rho = rho;
		p -> generate_maskpoly_after_rho(C.circuit[i - 1].bit_length * 2 + 1, 2);
		bool r_verify_cc = vpdR::check_commit(digest_maskR[0], digest_maskR[1]);
		bool msk_poly_cc = vpd_test::check_commit(digest_mask[0], digest_mask[1]);
		
		//add maskpoly

		alpha_beta_sum.value = (alpha_beta_sum.value + p->maskpoly_sumc.value) % prime_field::mod;

		p -> sumcheck_phase1_init();
		prime_field::field_element previous_random = prime_field::field_element(0);
		//next level random
		auto r_u = generate_randomness(C.circuit[i - 1].bit_length);
		auto r_v = generate_randomness(C.circuit[i - 1].bit_length);

		auto r_c = generate_randomness(1);
		prime_field::field_element *one_minus_r_u, *one_minus_r_v;
		one_minus_r_u = new prime_field::field_element[C.circuit[i - 1].bit_length];
		one_minus_r_v = new prime_field::field_element[C.circuit[i - 1].bit_length];
		
		for(int j = 0; j < C.circuit[i - 1].bit_length; ++j)
		{
			one_minus_r_u[j] = prime_field::field_element(1) - r_u[j];
			one_minus_r_v[j] = prime_field::field_element(1) - r_v[j];
		}

		for(int j = 0; j < C.circuit[i - 1].bit_length; ++j)
		{	
			if(j == C.circuit[i - 1].bit_length - 1){
				quintuple_poly poly = p->sumcheck_phase1_updatelastbit(previous_random, j);
				previous_random = r_u[j];
			

				if(poly.eval(0) + poly.eval(1) != alpha_beta_sum)
				{ 
					std::cout << "round j = " << j << std::endl;
					fprintf(stderr, "Verification fail, phase1, circuit %d, current bit %d\n", i, j);
					return false;
				}
				else
				{
					
				}
				alpha_beta_sum = poly.eval(r_u[j]);
			}

			else{
				quadratic_poly poly = p -> sumcheck_phase1_update(previous_random, j);
		
				previous_random = r_u[j];
			

				if(poly.eval(0) + poly.eval(1) != alpha_beta_sum)
				{
					std::cout << "round j = " << j << std::endl;
					fprintf(stderr, "Verification fail, phase1, circuit %d, current bit %d\n", i, j);
					return false;
				}
				else
				{
					
				}
				alpha_beta_sum = poly.eval(r_u[j]);
			}
		}
		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
		std::cerr << "	Time: " << time_span.count() << std::endl;
		
		std::cerr << "Bound v start" << std::endl;
		t0 = std::chrono::high_resolution_clock::now();
		p -> sumcheck_phase2_init(previous_random, r_u, one_minus_r_u);
		previous_random = prime_field::field_element(0);
		for(int j = 0; j < C.circuit[i - 1].bit_length; ++j)
		{
			if(j == C.circuit[i - 1].bit_length - 1){
				quintuple_poly poly = p -> sumcheck_phase2_updatelastbit(previous_random, j);
				previous_random = r_v[j];
				if(poly.eval(0) + poly.eval(1) != alpha_beta_sum)
				{
					fprintf(stderr, "Verification fail, phase2, circuit level %d, current bit %d\n", i, j);
					return false;
				}
				else
				{
					
				}
				alpha_beta_sum = poly.eval(r_v[j]);
			}
			else
			{
				quadratic_poly poly = p -> sumcheck_phase2_update(previous_random, j);
			
				previous_random = r_v[j];
				if(poly.eval(0) + poly.eval(1) != alpha_beta_sum)
				{
					fprintf(stderr, "Verification fail, phase2, circuit level %d, current bit %d\n", i, j);
					return false;
				}
				else
				{
					
				}
				alpha_beta_sum = poly.eval(r_v[j]);
			}
		}
		//Add one more round for maskR
		//quadratic_poly poly p->sumcheck_finalroundR(previous_random, C.current[i - 1].bit_length);

		t1 = std::chrono::high_resolution_clock::now();
		time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
		std::cerr << "	Time: " << time_span.count() << std::endl;
		
		auto final_claims = p -> sumcheck_finalize(previous_random);
		prime_field::field_element ttt = prime_field::field_element(1);
		
		auto v_u = final_claims.first;
		auto v_v = final_claims.second;
		
		beta_init(i, alpha, beta, r_0, r_1, r_u, r_v, one_minus_r_0, one_minus_r_1, one_minus_r_u, one_minus_r_v);
		auto mult_value = mult(i);
		auto add_value = add(i);
		
		quadratic_poly poly = p->sumcheck_finalround(previous_random, C.circuit[i - 1].bit_length << 1, add_value * (v_u + v_v) + mult_value * v_u * v_v);

		if(poly.eval(0) + poly.eval(1) != alpha_beta_sum)
		{
			fprintf(stderr, "Verification fail, phase2, lastbit for c\n");
			return false;
		}
		//previous_random = r_c[0];
		alpha_beta_sum = poly.eval(r_c[0]);

		mpz_class maskRg1_value_mpz, maskRg2_value_mpz;
		std::vector<mpz_class> r;
		r.resize(2);
		r[0] = p -> prepreu1.to_gmp_class(), r[1] = r_c[0].to_gmp_class();
		auto witnesses = p -> prove_R(r, maskRg1_value_mpz);
		prime_field::field_element tmp_rg1;
		bool r_verify_verify = vpdR::verify(r, digest_maskR[0], maskRg1_value_mpz, witnesses.first, witnesses.second);

		r[0] = p -> preprev1.to_gmp_class();
		witnesses = p -> prove_R(r, maskRg2_value_mpz);
		r_verify_verify &= vpdR::verify(r, digest_maskR[0], maskRg2_value_mpz, witnesses.first, witnesses.second);

		if(r_verify_verify & r_verify_cc)
		{
			fprintf(stderr, "VPD R pass\n");
		}
		else
		{
			fprintf(stderr, "VPD R failed\n");
			return false;
		}

		r.clear();
		for(int j = 0; j < C.circuit[i - 1].bit_length; ++j)
			r.push_back(r_u[j].to_gmp_class());
		for(int j = 0; j < C.circuit[i - 1].bit_length; ++j)
			r.push_back(r_v[j].to_gmp_class());
		r.push_back(r_c[0].to_gmp_class());
		mpz_class maskpoly_value_mpz = 0;
		witnesses = p -> prove_mask(r, maskpoly_value_mpz);
		auto msk_poly_verify = vpd_test::verify(r, digest_mask[0], maskpoly_value_mpz, witnesses.first, witnesses.second);
		

		if(msk_poly_cc & msk_poly_verify)
		{
			fprintf(stderr, "VPD mask pass\n");
		}
		else
		{
			fprintf(stderr, "VPD mask fail\n");
			return false;
		}
		
		prime_field::field_element maskpoly_value;
		prime_field::field_element maskRg1_value;
		prime_field::field_element maskRg2_value;
		maskpoly_value.value = prime_field::u512b(maskpoly_value_mpz.get_str().c_str(), maskpoly_value_mpz.get_str().length(), 10);
		maskRg1_value.value = prime_field::u512b(maskRg1_value_mpz.get_str().c_str(), maskRg1_value_mpz.get_str().length(), 10);
		maskRg2_value.value = prime_field::u512b(maskRg2_value_mpz.get_str().c_str(), maskRg2_value_mpz.get_str().length(), 10);
		if(alpha_beta_sum != r_c[0] * (add_value * (v_u + v_v) + mult_value * v_u * v_v) + alpha * p -> Iuv * p ->preZu * maskRg1_value + beta * p -> Iuv * p -> preZv * maskRg2_value + rho * maskpoly_value)
		{
			fprintf(stderr, "Verification fail, semi final, circuit level %d\n", i);
			return false;
		}
		else
		{
			fprintf(stderr, "Verification Pass, semi final, circuit level %d\n", i);
		}
		auto tmp_alpha = generate_randomness(1), tmp_beta = generate_randomness(1);
		alpha = tmp_alpha[0];
		beta = tmp_beta[0];
		delete[] tmp_alpha;
		delete[] tmp_beta;
		//v_u = v_u - p->Zu * p->sumRc.eval(p->preu1);
		//v_v = v_v - p->Zv * p->sumRc.eval(p->prev1);

		//std::cout << "test = " << alpha * p->Zu * p->sumRc.eval(p->preu1).to_string() << std::endl;  
		alpha_beta_sum = alpha * v_u + beta * v_v;

		delete[] r_0;
		delete[] r_1;
		delete[] one_minus_r_0;
		delete[] one_minus_r_1;
		r_0 = r_u;
		r_1 = r_v;
		one_minus_r_0 = one_minus_r_u;
		one_minus_r_1 = one_minus_r_v;
	}

	//post sumcheck

	auto input_0 = V_in(r_0, one_minus_r_0, input, C.circuit[0].bit_length, (1 << C.circuit[0].bit_length)), 
		 input_1 = V_in(r_1, one_minus_r_1, input, C.circuit[0].bit_length, (1 << C.circuit[0].bit_length));
	
	std::vector<mpz_class> r_0_mpz, r_1_mpz;
	for(int i = 0; i< C.circuit[0].bit_length; ++i)
		r_0_mpz.push_back(r_0[i].to_gmp_class());
	for(int i = 0; i< C.circuit[0].bit_length; ++i)
		r_1_mpz.push_back(r_1[i].to_gmp_class());
	
	mpz_class input_0_mpz, input_1_mpz;

	input_0_mpz = 0, input_1_mpz = 0;
	auto witnesses_0 = p -> prove_input(r_0_mpz, input_0_mpz, p -> Zu.to_gmp_class());
	auto witnesses_1 = p -> prove_input(r_1_mpz, input_1_mpz, p -> Zv.to_gmp_class());
	cout << input_0_mpz << endl;
	cout << input_0.to_gmp_class() << endl;

	cout << input_1_mpz << endl;
	cout << input_1.to_gmp_class() << endl;

	bool input_0_verify = input_vpd::verify(r_0_mpz, digest_input.first[0], digest_input.second[0], p -> Zu.to_gmp_class(), input_0_mpz, witnesses_0.first, witnesses_0.second);
	bool input_1_verify = input_vpd::verify(r_1_mpz, digest_input.first[0], digest_input.second[0], p -> Zv.to_gmp_class(), input_1_mpz, witnesses_1.first, witnesses_1.second);
	cout << "Verify result: " << input_0_verify << " " << input_1_verify << endl;

	input_0 = input_0 + p->Zu * p->sumRc.eval(p->preu1);
	input_1 = input_1 + p->Zv * p->sumRc.eval(p->prev1);

	cout << input_0_mpz << endl;
	cout << input_0.to_gmp_class() << endl;

	delete[] input;
	delete[] r_0;
	delete[] r_1;
	delete[] one_minus_r_0;
	delete[] one_minus_r_1;
	if(alpha_beta_sum != input_0 * alpha + input_1 * beta)
	{
		fprintf(stderr, "Verification fail, final input check fail.\n");
		return false;
	}
	else
	{
		fprintf(stderr, "Verification pass\n");
		std::cerr << "Prove Time " << p -> total_time << std::endl;
	}
	p -> delete_self();
	delete_self();
	return true;
}

void zk_verifier::delete_self()
{
	delete[] beta_g_r0;
	delete[] beta_g_r1;
	delete[] beta_u;
	delete[] beta_v;
	//std::cout << "come here!" << std::endl;
	for(int i = 0; i < C.total_depth; ++i)
	{
		delete[] C.circuit[i].gates;
	}
	delete[] C.circuit;
}
