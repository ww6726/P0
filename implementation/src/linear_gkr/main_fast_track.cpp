#define __debug__
#define __timer__
#include "linear_gkr/verifier_fast_track.h"
#include "linear_gkr/prover_fast_track.h"
#include "linear_gkr/prime_field.h"
#include "linear_gkr/prime_field_small.h"

#include <iostream>
#include <cassert>
using namespace std;
verifier v;
prover p;



void inference()
{	
	double* outputs = p.do_something();
	int size[4] = {28*28,128,64,8};

	std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();
	cout<<"===============CONVOLUTION fisrt=================="<<endl;
	//test conv
	for(int i =0;i<6;i++){
		v.read_input_and_kernel(28,5,0,6);//matrix size, kernel size, padding,innumChannel
		p.get_matrices(v.A,v.B,v.matrix_size);
		printf("%s\n", v.verify_matrix() ? "Pass" : "Fail");
	}
	exit(1);
	cout<<"===============POOLING first=================="<<endl;

	// pool 1
	prime_field::field_element* inputs = new prime_field::field_element[8];
	for (int j = 0; j < 8; ++j)
	{
		inputs[j] = 3;
	}
	 v.create_maxpool_circuit(inputs,8);
	 p.get_circuit(v.C);
	 bool result_maxpool = v.verify();
	
	cout<<"===============CONVOLUTION second=================="<<endl;

	//conv2 
	for(int i =0;i<16;i++){
		v.read_input_and_kernel(14,5,0,6);//matrix size, kernel size, padding,innumChannel
		p.get_matrices(v.A,v.B,v.matrix_size);
		printf("%s\n", v.verify_matrix() ? "Pass" : "Fail");
	}

	// //pool 2
	cout<<"===============POOLING second=================="<<endl;

	 v.create_maxpool_circuit(inputs,64);
	 p.get_circuit(v.C);
	 result_maxpool = v.verify();
	// //pool 3

	cout<<"===============Fully connected layer 1=================="<<endl;
	//linear1
	 v.read_input_matrices(128);
    p.get_matrices(v.A,v.B,v.matrix_size);
    printf("%s\n", v.verify_matrix() ? "Pass" : "Fail");
	//linear2 
	cout<<"===============Fully connected layer 2=================="<<endl;

	 v.read_input_matrices(64);
    p.get_matrices(v.A,v.B,v.matrix_size);
    printf("%s\n", v.verify_matrix() ? "Pass" : "Fail");
	//linear3
	cout<<"===============Fully connected layer 3=================="<<endl;

	v.read_input_matrices(8);
    p.get_matrices(v.A,v.B,v.matrix_size);
    printf("%s\n", v.verify_matrix() ? "Pass" : "Fail");
	cout<<"[";
	for (int i = 0; i < 10; ++i)
    {
        cout<<outputs[i]<<" ";
    }
    cout<<"]"<<endl;
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
	std::cerr << "Inference time: " << time_span.count() << " seconds." << std::endl;
	
	
	
}


pair<int, vector<int>> server_setup(){
     // Server setup
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    int reuse = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 2); // Allow up to 2 clients to queue for connection

    std::cout << "Server listening for incoming connections...\n";

    std::vector<int> clientSockets;

    while (clientSockets.size() < 2) {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int newClientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

        std::cout << "New client connected.\n";
        clientSockets.push_back(newClientSocket);
    }
    return make_pair(serverSocket,clientSockets);
}
void send_int(vector<int>& data, int client)
{
    send(client, data.data(), sizeof(int)*data.size(), 0);
}
vector<int> recv_int(int client, int n)
{	
    vector<int> received_data(n);
    recv(client, received_data.data(), sizeof(int)*n, 0);
    return received_data;
}
void send_field(vector<prime_field::field_element> data, int client)
{
    send(client, &data, sizeof(prime_field::field_element)*data.size(), 0);
}
vector<prime_field::field_element> recv_field(int client, int n)
{	
    vector<prime_field::field_element> received_data(n);
    recv(client, &received_data, sizeof(prime_field::field_element)*n, 0);
    return received_data;
}
int main(int argc, char** argv)
{

	// prime_field::init("16798108731015832284940804142231733909759579603404752749028378864165570215949", 10);
	prime_field::init("2305843009213693951", 10);
	p.total_time = 0;
	int id = atoi(argv[1]);
	p.prover_id = id;
	// 	//test 
	// int size = 4;
	// prime_field::field_element* inputs = new prime_field::field_element[size];
	// for(int i=0;i<size;i++){
	// 	inputs[i] = prime_field::field_element(i);
	// }
	// v.create_maxPool_circuit(inputs,size);
	// exit(1);
	v.get_prover(&p);
	// cout<<"TESTING GKR"<<endl;
	// v.read_circuit("simple_circuit.txt");
	// v.read_circuit("mat_16_circuit.txt");

	

	

	//setup server
	pair<int,vector<int>> server_clients = server_setup();
	p.server = server_clients.first;
	p.clients = server_clients.second;


    v.read_input_matrices(128);
    p.get_matrices(v.A,v.B,v.matrix_size);
    bool result_mat_mul = v.verify_matrix();
    printf("%s\n", result_mat_mul ? "Pass" : "Fail");
    exit(1);


	//circuit
    v.read_circuit("mat_16_circuit.txt");
	p.get_circuit(v.C);
	bool result = v.verify();
	printf("%s\n", result ? "Pass" : "Fail");
	return 0;
}