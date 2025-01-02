/*
#pragma omp master
{
	cout << "Number of threads = " << omp_get_num_threads() << endl;
}
//omp_set_dynamic(0);
//omp_set_num_threads(omp_get_num_procs() - 1);
*/

vector<uint32_t> par_randomized_cc(uint32_t nNodes, vector<Edge>& edges, vector<uint32_t>& labels) {
	if(edges.size() == 0 || nNodes == 0) 
		return labels;

	vector<bool> coin_toss(nNodes);
	vector<Edge> nextEdges;
	vector<uint32_t> nextLabels(nNodes);
	
	vector<uint32_t> s(edges.size(), 0);
	vector<uint32_t> S(edges.size(), 0);

	#pragma omp parallel 
	{	
		// Mersenne Twister 19937 generator
		// Note: this need to be private to each thread because
		// std::mt19937 is not thread-safe
		mt19937 random_engine(seed + omp_get_thread_num());

		// Generate random coin tosses
		#if DEBUG
		#pragma omp master 
		#else
		#pragma omp for
		#endif
		for (uint32_t i = 0; i < nNodes; i++) {
			coin_toss[i] = random_engine() % 2;
		}
		#pragma omp barrier

		// hook child to a parent
		//#pragma omp for schedule(static)
		#if DEBUG
		#pragma omp single
		#else
		#pragma omp for
		#endif
		for(auto it = edges.begin(); it < edges.end(); it++)
		{
			// Tail is True and Head is False
			if(coin_toss[it->from] && !coin_toss[it->to])
			{
				#pragma omp atomic write
				labels[it->from] = labels[it->to];
			}
		}
		
		#pragma omp for
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			// Prepare to remove intra-group edges
			if(labels[edges[i].from] != labels[edges[i].to])
				s[i] = 1;
		}

		// Prefix sum TODO: parallelize
		#pragma omp single
		{
			// Prefix sum
			S[0] = s[0];
			for(uint32_t i = 1; i < edges.size(); i++)
			{
				S[i] += s[i] + S[i - 1];
			}
		}

		// Allocate memory for nextEdges array with the size of the last element of S
		#pragma omp sigle
		{
			nextEdges.resize(S[edges.size() - 1]);
		}

		// Copy only the inter-group edges to nextEdges
		// copy only edges that are between different groups
		#if DEBUG
		#pragma omp single
		#else
		#pragma omp for
		#endif
		for(uint32_t i = 0; i < edges.size(); i++)
		{
			if(labels[edges[i].from] != labels[edges[i].to])
				nextEdges[S[i]] = Edge{labels[edges[i].from], labels[edges[i].to]};
		}
	}

	// Recursively call the function
	//par_randomized_cc(nNodes, nextEdges, labels);


	//Map results back to the original graph
	#pragma omp parallel for
	for(uint32_t i = 0; i < edges.size(); i++)
	{
		if(edges[i].from == labels[edges[i].to])
		{
			#pragma omp atomic write
			labels[edges[i].from] = labels[edges[i].to];
		}
	}

	/*
	// Print the coin tosses
	cout << "Coin tosses: ";
	for (uint32_t i = 0; i < nNodes; i++) {
		cout << coin_toss[i] << " ";
	}
	cout << endl;

	// Print the s array
	cout << "s array: ";
	for (uint32_t i = 0; i < edges.size(); i++) {
		cout << "(" << edges[i].from << "," << edges[i].to << ")->" << s[i] << " ";
	}
	cout << endl;

	// Print the S array
	cout << "S array: ";
	for (uint32_t i = 0; i < edges.size(); i++) {
		cout << "(" << edges[i].from << "," << edges[i].to << ")->" << S[i] << " ";
	}
	cout << endl;
	*/

	return labels;
}