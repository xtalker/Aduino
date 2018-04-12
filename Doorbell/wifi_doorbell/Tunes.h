
// Define tune sequences
int tune_1 [][4] = { 
  // Tone mapping for a tune:
  //  Low,  Med,  Hi  Len  
  {   1,    0,    0,  1000 }, 
  {   0,    1,    0,  1000 }, 
  {   0,    0,    1,  1000 }, 
  {   1,    1,    0,  1000 }, 
  {   0,    0,    0,  0    },
  {   0,    1,    1,  500  }, 
  {   1,    0,    1,  500  },
  {   1,    1,    1,  500  }
};

int startup [][4] = { 
  // Tone mapping for a tune:
  //  Low,  Med,  Hi  Len  
  {   1,    0,    0,  100 }, 
  {   0,    1,    0,  100 }, 
  {   0,    0,    1,  100 }, 
};

