#include "adapters/web/WebSearchAdapter.h"
#include "adapters/maps/MapsAdapter.h"
#include <cassert>
#include <iostream>
int main(){auto w=core::adapters::web::googleUrl("hello world");assert(w.find("hello%20world")!=std::string::npos);auto m=core::adapters::maps::directionsUrl("A","B");assert(m.find("origin=A")!=std::string::npos&&m.find("destination=B")!=std::string::npos);std::cout<<"Adapter tests passed\n";}
