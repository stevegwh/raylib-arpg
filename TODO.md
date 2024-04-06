## To Do

- Every call to ECS->system should be replaced with a pointer to that system passed into the object.
  - This should enforce a strict order of initialisation and make dependencies clearer.