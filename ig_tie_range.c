// gcc ig_tie_range.c -I/usr/local/include/igraph -L/usr/local/lib -ligraph -o ig_tie_range
#include <igraph.h>


int print_matrix(const igraph_matrix_t *m) {
  long int nrow=igraph_matrix_nrow(m);
  long int ncol=igraph_matrix_ncol(m);
  long int i, j;
  for (i=0; i<nrow; i++) {
    printf("%li:", i);
    for (j=0; j<ncol; j++) {
      printf(" %3.0F", MATRIX(*m, i, j));
    }
    printf("\n");
  }
  return 0;
}

void print_vector(igraph_vector_t *v) {
  long int i;
  for (i=0; i<igraph_vector_size(v); i++) {
    printf(" %li, ", (long int) VECTOR(*v)[i]);
  }
}

void print_vector_censor(igraph_vector_t *v, long int no_print) {
  long int i;
  long int res;
  for (i=0; i<igraph_vector_size(v); i++) {
  	res = (long int) VECTOR(*v)[i];
  	if (res != no_print) {
	    printf(" %li, ", res);
	}
  }
}

void print_strvector(igraph_strvector_t *v) {
  if (igraph_strvector_size(v) < 1) {
  	printf("None\n");
  }
  long int i;
  for (i=0; i<igraph_strvector_size(v); i++) {
    printf(" %s\n", STR(*v,i));
  }
}
 
int tie_range(const igraph_t *g, igraph_integer_t eid) {
	igraph_integer_t s_node;
	igraph_integer_t d_node;
	igraph_vector_t  visited;
	igraph_vector_t  node_queue;
	igraph_vector_t  neighbors;
	long int vcount;
	long int wave_end;
	long int wave_begin;
	long int last_pos;
	long int i;
	long int step;
	long int j;
	int found;
	
	
	// compute source and destination
	igraph_edge( g, eid, &s_node, &d_node );
	
	// track node visits, init to zero
	vcount = (long int)igraph_vcount(g);
	igraph_vector_init(&visited, vcount);
	igraph_vector_init(&node_queue, vcount);
	igraph_vector_fill(&node_queue, -1);
	
    // Set up initial step, skipping the null_edge
    igraph_vector_init( &neighbors, 0 );
    igraph_neighbors(g, &neighbors, s_node, IGRAPH_ALL);
    
    igraph_vector_search( &neighbors, 0, d_node, &i);
    igraph_vector_remove( &neighbors, i );
    
    // Prepare tracking variables
    wave_end = 0;
    wave_begin = 0;
    last_pos = 0;
    found = 0;
    
    VECTOR(visited)[ (int)s_node ] = 1;
    for (i=0; i<igraph_vector_size(&neighbors); i++) {
    	VECTOR(visited)[ (int)(VECTOR(neighbors)[i]) ] = 1;
    	VECTOR(node_queue)[i] = (VECTOR(neighbors)[i]);
    	wave_end += 1;
    	last_pos += 1;
    }
    
    //printf("S_node: %i, D_node: %i\n", s_node, d_node);
    //printf("Wave ids: %li, %li\n", wave_begin, wave_end); 
    //print_vector(&node_queue);
    
    step = 1;
    // BFS through graph until target node is reached
    while ( wave_end > wave_begin ) {
		step ++;
		//print_vector_censor(&node_queue, -1L);
		//printf("\n");
    	for (j=wave_begin; j<wave_end; j++) {
    		igraph_neighbors(g, &neighbors, VECTOR(node_queue)[j], IGRAPH_ALL);
    		//printf("Step: %li:%li found %li neighbors\n", step, j, igraph_vector_size(&neighbors));
    		//print_vector(&neighbors);
    		//printf("\n");
			for (i=0; i<igraph_vector_size(&neighbors); i++) {
				if ( VECTOR(neighbors)[i] == d_node ) {
					//printf("Found D_Node: %i\n", (int)VECTOR(neighbors)[i]);
					found = 1;
					break;		
				}
				else if ( VECTOR(visited)[( long int)(VECTOR(neighbors)[i]) ] == 0 ) {			
					VECTOR(visited)[ (int)(VECTOR(neighbors)[i]) ] = 1;
					VECTOR(node_queue)[last_pos] = (VECTOR(neighbors)[i]);
					last_pos += 1;
				}
    		}
			if (found) { break;}
		}
		if (found) {break;}
		wave_begin = wave_end;
		wave_end = last_pos;
    }
    if (!found) {
    	step = -1;
    }
  // Clean up
  igraph_vector_destroy(&visited);
  igraph_vector_destroy(&node_queue);
  igraph_vector_destroy(&neighbors);    
  return step;
}

int main(int argc, char *argv[]) {
  igraph_t g;
  igraph_eit_t eid_iter;
  igraph_integer_t eid;
  FILE *in_file, *out_file;
  int i;
  int result;
  
  /*
  igraph_strvector_t gnames;
  igraph_vector_t    gtypes;
  igraph_strvector_t vnames;
  igraph_vector_t    vtypes;
  igraph_strvector_t enames;
  igraph_vector_t    etypes;
  */
  
  
  // Enable C graph attributes
  igraph_i_set_attribute_table(&igraph_cattribute_table);
  
  in_file = fopen(argv[1], "r");
  out_file = fopen(argv[2], "w");
  
  // See if the files exist
  if (in_file==0) {
  	printf("Could not open in file\n");
  	return 1;
  }
  if (out_file==0) {
  	printf("Could not open out file\n");
  	return 1;
  }
  
  // Read in the graphml
  if ( (result=igraph_read_graph_graphml(&g, in_file, 0)) ) {
  	printf("Trouble reading format\n");
  	return 1;
  }
  fclose(in_file);
  
  /*
	igraph_strvector_init( &gnames, 0 );
	igraph_vector_init( &gtypes, 0 );
	igraph_strvector_init( &vnames, 0 );
	igraph_vector_init( &vtypes, 0 );
	igraph_strvector_init( &enames, 0 );
	igraph_vector_init( &etypes, 0 );
  
    printf("Getting Attribs\n");
    	
	eid = igraph_cattribute_list(&g, &gnames, &gtypes, &vnames, &vtypes, &enames, &etypes);
	
	printf("Attrib Query Returned: %i\n",eid);
	print_strvector(&vnames);
	printf("\n");
	print_strvector(&enames);
  */
  
  /*
  // Test a single edge
  eid =  30902;
  result = tie_range( &g, eid );
  printf("Tie Range for edge %i is %i\n", eid, result);
  */
  
  
  // For each edge, what is its range?
  
  // Create edge iterator
  igraph_eit_create( &g, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eid_iter);
  
  // Iterate and find tie_range, store in edge attribute range
  while ( !IGRAPH_EIT_END(eid_iter) ) {
  	eid = IGRAPH_EIT_GET(eid_iter);
  	SETEAN(&g, "range", eid, tie_range( &g, eid )); 
  	//printf("%i\n", eid);
  	IGRAPH_EIT_NEXT(eid_iter);
  }

  igraph_eit_destroy( &eid_iter);
  
  // Write it back
  if (out_file) {
  	if ( (result=igraph_write_graph_graphml(&g, out_file)) ) {
  		return 1;
  	}
  	fclose(out_file);
  }
  
  /*
  printf("The undirected graph:\n");
  printf("Vertices: %li\n", (long int) igraph_vcount(&g));
  printf("Edges: %li\n", (long int) igraph_ecount(&g));
  printf("Directed: %i\n", (int) igraph_is_directed(&g));
  */
  
  // Clean up
  igraph_destroy(&g);
  
  return 0;
}  

/*
  igraph_real_t avg_path;
  igraph_t graph;
  igraph_vector_t dimvector;
  igraph_vector_t edges;
  igraph_matrix_t path_lens;
  igraph_vs_t from;
  igraph_vs_t to;
  igraph_integer_t vid;
  long int avg_pathlen = 0;
  int pos_node_paths = 0;
  
  int i,j;
  
  vid = 0;
  
  igraph_matrix_init(&path_lens, 0, 0);   //path_lens= igraph_matrix_init(0, 0)
  igraph_vector_init(&dimvector, 2);
  
  VECTOR(dimvector)[0]=60;     // dimvector[0] = 30
  VECTOR(dimvector)[1]=60;
  igraph_lattice(&graph, &dimvector, 4, IGRAPH_UNDIRECTED, 0, 1);

  srand(100);
  igraph_vector_init(&edges, 20);
  for (i=0; i<igraph_vector_size(&edges); i++) {
    VECTOR(edges)[i] = rand() % (int)igraph_vcount(&graph);
  }

  
  igraph_shortest_paths(&graph, &path_lens, igraph_vss_all(), igraph_vss_all(), IGRAPH_ALL);
  for (i=0; i<(int)igraph_vcount(&graph); i++) {
    for (j=0; j<(int)igraph_vcount(&graph); j++) {
       avg_pathlen += (long int) MATRIX(path_lens, i, j);
    }
  }
  pos_node_paths = (int)igraph_vcount(&graph) * ((int)igraph_vcount(&graph)-1);
  
  printf("Average path length (lattice):            %f\n", (double)avg_pathlen/(double)pos_node_paths );  
  
  igraph_average_path_length(&graph, &avg_path, IGRAPH_UNDIRECTED, 1);
  printf("Average path length (lattice):            %f\n", (double) avg_path);

  //print_matrix(&path_lens);
  
  igraph_add_edges(&graph, &edges, 0);
  igraph_average_path_length(&graph, &avg_path, IGRAPH_UNDIRECTED, 1);
  printf("Average path length (randomized lattice): %f\n", (double) avg_path);
  
  igraph_vector_destroy(&dimvector);
  igraph_vector_destroy(&edges);
  igraph_matrix_destroy(&path_lens);
  igraph_destroy(&graph);

  return 0;
}
*/