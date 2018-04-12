
  // initialize digital outputs.
  pinMode(CHIME_LOW, OUTPUT);
  pinMode(CHIME_MED, OUTPUT);
  pinMode(CHIME_HI,  OUTPUT);
  pinMode(LED,       OUTPUT);
  pinMode(NODE_LED,  OUTPUT);
  
  digitalWrite(CHIME_LOW, LOW);
  digitalWrite(CHIME_MED, LOW);
  digitalWrite(CHIME_HI,  LOW);
  digitalWrite(LED,       LOW);
  digitalWrite(NODE_LED,  HIGH);

  // initialize digital inputs.
  pinMode(FRONT_DOOR,  INPUT_PULLUP);
  pinMode(BACK_DOOR,   INPUT_PULLUP);
  pinMode(CTRL_BUTTON, INPUT_PULLUP);


