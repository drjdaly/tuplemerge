// File: trace_tools.cc
// David E. Taylor 
// Applied Research Laboratory
// Department of Computer Science and Engineering
// Washington University in Saint Louis
// det3@arl.wustl.edu
//
// Functions for generating synthetic trace of headers
//

#include "trace_tools.h"


// Generate headers
// a,b in ClassBench are 1 0.1 
// generate at least 'threshold' number of packets
// To ensure the generated dataset is deterministic, call this first!!
std::vector<Packet> header_gen(int d, std::vector<Rule>& filters, float a, float b, int threshold){
  int num_headers = 0;
  int fsize = filters.size();

  std::vector<Packet> temp_packets;

  // Allocate temporary header
  unsigned *new_hdr = new unsigned[d];

  // Generate headers
  while(num_headers < threshold){
    // Pick a random filter
    int RandFilt = Random::random_int(0,fsize-1);

    // Pick a random corner of the filter for a header
    RandomCorner(RandFilt, filters, new_hdr,d);

    // Select number of copies to add to header list
    // from Pareto distribution
    int Copies = MyPareto(a,b);
    // printf("RandFilt = %d, a = %.4f, b = %.4f, Copies = %d\n",RandFilt,a,b,Copies);

    // Add to header list
	std::vector<unsigned> temp;
	for (int i = 0; i < d; i++) temp.push_back(new_hdr[i]);
	for (int i = 0; i < Copies; i++)  {
		temp_packets.push_back(temp);
	}
    // Increment number of headers
    num_headers += Copies;
  }

  delete(new_hdr);
  return std::vector<Packet>(begin(temp_packets), begin(temp_packets)+threshold);
}
void RandomCorner(int RandFilt, std::vector<Rule>& filts, unsigned* new_hdr, int d){

  // Random number
	double p;

  for (int i = 0; i < d; i++){
	  p = Random::random_real_btw_0_1();
    // Select random number
    if (p < 0.5){
      // Choose low extreme of field
		new_hdr[i] = filts[RandFilt].range[i][0]; 
    } else {
      // Choose high extreme of field
		new_hdr[i] = filts[RandFilt].range[i][1];
    }
  }
  return;
}

int MyPareto(float a, float b){
  if (b == 0) return 1;

  // Random number
  double p;
  // Select random number
  p = Random::random_real_btw_0_1();
 
  double x = (double)b / pow((double)(1 - p),(double)(1/(double)a));
  int Num = (int)ceil(x);
  return Num;
}


std::vector<Packet> GeneratePacketsFromRuleset(std::vector<Rule>& filters, int num_packets){
	if (filters.empty()) printf("warning there is no rule?\n");
	return header_gen(filters[0].dim, filters, 1, 0.1f, num_packets);
}