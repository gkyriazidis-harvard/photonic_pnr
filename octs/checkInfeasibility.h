
// This code should check and find which constraints violate the optimization target

//Once you find the constraints, you should find the points that correspond to these constraints

// Afterwards you need to find the reason
// Only reason for timing violations is a high delay on one side of the tree, while for 1x3 1x5 splitters it can be that few MRs do not overlap with each other
// For 1x2 it is simple
// For higher you should try to find how to make all the MRs overlap with each other

// Add the spiral for each scenario (place) and using the altered delay rerun the optimization
// Do not forget to add the spiral in the routing stage

void checkInfeasibility() {

}

