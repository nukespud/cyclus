// GenericRepository.cpp
// Implements the GenericRepository class
#include <iostream>

#include "GenericRepository.h"

#include "Logician.h"
#include "GenException.h"
#include "InputXML.h"
#include "Timer.h"



/**
 * The GenericRepository class inherits from the FacilityModel class and is dynamically
 * loaded by the Model class when requested.
 * 
 * This facility model is intended to calculate nuclide and heat metrics over
 * time in the repository. It will make appropriate requests for spent fuel 
 * which derive from heat- and perhaps dose- limited space availability. 
 *
 * BEGINNING OF SIMULATION
 * At the beginning of the simulation, this facility model loads the 
 * components within it and figures out its initial capacity for each heat or 
 * dose generating waste type it expects to accept. 
 *
 * TICK
 * Examining the stocks, materials recieved last month are emplaced.
 * The repository determines its current capacity for the first of the 
 * incommodities (waste classifications?) and requests as much 
 * of the incommodities that it can fit. The next incommodity is on the docket 
 * for next month. 
 *
 * TOCK
 * The repository passes the Tock radially outward through its 
 * components.
 *
 * (r = 0) -> -> -> -> -> -> -> ( r = R ) 
 * mat -> form -> package -> buffer -> barrier -> near -> far
 *
 * RECEIVE MATERIAL
 * Put the material in the stocks
 *
 * RECEIVE MESSAGE
 * reject it, I don't do messages.
 *
 *
 */


 
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void GenericRepository::init(string name, vector<Commodity*> in_commods, 
      double capacity, int lifetime, double area, int startOpYr, int startOpMo){
  //FacilityModel::init(name, impl, etc.))
  vector<Commodity*>::iterator it;
  for ( it=in_commods.begin() ; it < in_commods.end(); it++ ){
    in_commods_.push_back(*it) ;
  }
  capacity_ = capacity;
  lifetime_ = lifetime;
  area_ = area;
  startOpYr_ = startOpYr;
  startOpMo_ = startOpMo;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void GenericRepository::init(xmlNodePtr cur)
{ 
  FacilityModel::init(cur);
  
  // move XML pointer to current model
  cur = XMLinput->get_xpath_element(cur,"model/GenericRepository");

  // initialize ordinary objects
  capacity_ = atof(XMLinput->get_xpath_content(cur,"capacity"));
  lifetime_ = atof(XMLinput->get_xpath_content(cur,"lifetime"));
  area_ = atof(XMLinput->get_xpath_content(cur,"area"));
  inventory_size_ = atof(XMLinput->get_xpath_content(cur,"inventorysize"));
  startOpYr_ = atoi(XMLinput->get_xpath_content(cur,"startOperYear"));
  startOpMo_ = atoi(XMLinput->get_xpath_content(cur,"startOperMonth"));

  // The repository accepts any commodities designated waste.
  // This will be a list
  //

  /// all facilities require commodities - possibly many
  string commod_name;
  Commodity* new_commod;
  xmlNodeSetPtr nodes = XMLinput->get_xpath_elements(cur,"incommodity");

  for (int i=0;i<nodes->nodeNr;i++)
  {
    commod_name = (const char*)(nodes->nodeTab[i]->children->content);
    new_commod = LI->getCommodity(commod_name);
    if (NULL == new_commod)
      throw GenException("Input commodity '" + commod_name 
          + "' does not exist for facility '" + getName() 
                            + "'.");
    in_commods_.push_back(new_commod);
  }

  stocks_ = deque<Material*>();
  inventory_ = deque< Material* >();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GenericRepository::copy(GenericRepository* src)
{

  FacilityModel::copy(src);

  // are these accessing the right stuff?
  capacity_ = src->capacity_;
  inventory_size_ = src->inventory_size_;
  startOpYr_ = src->startOpYr_;
  startOpMo_ = src->startOpMo_;
  in_commods_ = src->in_commods_;

  stocks_ = deque<Material*>();
  inventory_ = deque< Material*>();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void GenericRepository::copyFreshModel(Model* src)
{
  copy(dynamic_cast<GenericRepository*>(src));
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void GenericRepository::print() 
{ 
  FacilityModel::print(); 
  cout << "stores commodity {"
    << in_commods_.front()->getName()
      << "} among others."  << endl;
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void GenericRepository::receiveMessage(Message* msg)
{
  throw GenException("GenericRepository doesn't know what to do with a msg.");
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void GenericRepository::receiveMaterial(Transaction trans, vector<Material*> manifest)
{
  // grab each material object off of the manifest
  // and move it into the stocks.
  for (vector<Material*>::iterator thisMat=manifest.begin();
       thisMat != manifest.end();
       thisMat++)
  {
    cout<<"GenericRepository " << getSN() << " is receiving material with mass "
        << (*thisMat)->getTotMass() << endl;
    stocks_.push_front(*thisMat);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void GenericRepository::handleTick(int time)
{
  // EMPLACE WASTE
  emplaceWaste();

  // MAKE A REQUEST
  if(this->checkStocks() == 0){
    // It chooses the next incommodity in the preference lineup
    Commodity* in_commod;
    in_commod = in_commods_.front();

    // It then moves that commodity from the front to the back of the preference lineup
    in_commods_.push_back(in_commod);
    in_commods_.pop_front();
  
    // It can accept amounts however small
    Mass requestAmt;
    Mass minAmt = 0;
    // The GenericRepository should ask for material unless it's full
    Mass inv = this->checkInventory();
    // including how much is already in its stocks
    Mass sto = this->checkStocks(); 
    // this will be a request for free stuff
    double commod_price = 0;
    // subtract inv and sto from inventory max size to get total empty space
    Mass space = inventory_size_- inv - sto;
  
    if (space == 0){
      // don't request anything
    }
    else if (space <= capacity_){
      Communicator* recipient = dynamic_cast<Communicator*>(in_commod->getMarket());
      // if empty space is less than monthly acceptance capacity
      requestAmt = space;
      // recall that requests have a negative amount
      Message* request = new Message(UP_MSG, in_commod, -requestAmt, minAmt,
                                       commod_price, this, recipient);
        // pass the message up to the inst
        (request->getInst())->receiveMessage(request);
    }
    // otherwise
    else if (space >= capacity_){
      Communicator* recipient = dynamic_cast<Communicator*>(in_commod->getMarket());
      // the upper bound is the monthly acceptance capacity
      requestAmt = capacity_;
      // recall that requests have a negative amount
      Message* request = new Message(UP_MSG, in_commod, -requestAmt, minAmt, commod_price,
          this, recipient); 
      // send it
      sendMessage(request);
    };
  };
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void GenericRepository::handleTock(int time)
{
  // send tock to the emplaced material module
  
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
Mass GenericRepository::checkInventory(){
  Mass total = 0;

  // Iterate through the inventory and sum the amount of whatever
  // material unit is in each object.
  for (deque< Material*>::iterator iter = inventory_.begin(); 
       iter != inventory_.end(); 
       iter ++){
    total += (*iter)->getTotMass();
  }

  return total;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
Mass GenericRepository::checkStocks(){
  Mass total = 0;

  // Iterate through the stocks and sum the amount of whatever
  // material unit is in each object.

  if(!stocks_.empty()){
    for (deque< Material* >::iterator iter = stocks_.begin(); 
         iter != stocks_.end(); 
         iter ++){
        total += (*iter)->getTotMass();
    };
  };
  return total;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void emplaceWaste(){
  // EMPLACE THE WASTE
}

