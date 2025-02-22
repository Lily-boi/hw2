#include "mydatastore.h"
#include <iostream>
#include <fstream>
#include <algorithm>

MyDataStore::MyDataStore() {}

MyDataStore::~MyDataStore() { //Destructor stuff (cycle through products and users to delete)
  for (Product* p : products_) {
    delete p;
  }

  for (auto& user : users_) {
    delete user.second;
  }
}

void MyDataStore::addProduct(Product* p) {
  products_.push_back(p);
  std::set<std::string> keywords = p->keywords();

  for (const std::string& keyword : keywords) {
    keywordIndex_[convToLower(keyword)].insert(p);
  }
}

void MyDataStore::addUser(User* u) {
  std::string username = convToLower(u->getName());
  users_[username] = u;
  carts_[username] = std::deque<Product*>();
}

std::vector<Product*> MyDataStore::search(std::vector<std::string>& terms, int type) {
  std::set<Product*> resultSet;

  if (terms.empty()) {
    return std::vector<Product*>();
  }

  bool first = true; //Have to look for first term to have something to compare stuff with
  for (const std::string& term : terms) {
    auto it = keywordIndex_.find(convToLower(term));
    
    std::set<Product*> keywordMatches;

    if (it != keywordIndex_.end()) {
      keywordMatches = it->second;
    } else {
      keywordMatches = std::set<Product*>();
    }


    if (first) {
      resultSet = keywordMatches;
      first = false;
    } else {
      if (type == 0) { //If its AND do intersection
        resultSet = setIntersection(resultSet, keywordMatches);
      } else { //Otherwise its OR and do union
        resultSet = setUnion(resultSet, keywordMatches);
      }
    }
  }

  return std::vector<Product*>(resultSet.begin(), resultSet.end());
}


void MyDataStore::dump(std::ostream& ofile) {
  ofile << "<products>" << std::endl;
  for (Product* p : products_) {
    p->dump(ofile);
  }

  ofile << "</products>" << std::endl;

  ofile <<"<users>" << std::endl;

  for (auto& user : users_) {
    user.second->dump(ofile);
  }
  ofile << "</users>" << std::endl;
}

void MyDataStore::addToCart(const std::string& username, int hitIndex, std::vector<Product*>& searchResults) {
  std::string lowerUsername = convToLower(username);
  if (users_.find(lowerUsername) == users_.end()) { //If invalid username, just return
    std::cout << "Invalid Username" << std::endl;
    return;
  } 
  if (hitIndex < 0 || hitIndex >= static_cast<int>(searchResults.size())) { //If invalid index, return
    std::cout << "Invalid Index" << std::endl;
    return;
  }

  carts_[lowerUsername].push_back(searchResults[hitIndex]); //Add index to cart under username
}

void MyDataStore::viewCart(const std::string& username) {
  std::string lowerUsername = convToLower(username);
  if (users_.find(lowerUsername) == users_.end()) { //If invalid username, just return
    std::cout << "Invalid Username" << std::endl;
    return;
  } 

  int ind = 0;
  for (Product* p : carts_[lowerUsername]) { //Cycle through and print everything in cart
    std::cout << "Item " << ind + 1 << ": " << p->displayString() << "\n" << std::endl;
    ind++;
  }
}

void MyDataStore::buyCart(const std::string& username) {
  std::string lowerUsername = convToLower(username);
  if (users_.find(lowerUsername) == users_.end()) { //If invalid username, just return
    std::cout << "Invalid Username" << std::endl;
    return;
  } 

  std::deque<Product*>& cart = carts_[lowerUsername];
  User* user = users_[lowerUsername];

  std::deque<Product*> newCart; //Set up temp cart to hold items that are too expensive
  while (!cart.empty()) {
    Product* p = cart.front();
    cart.pop_front();

    if (p->getQty() > 0 && user->getBalance() >= p->getPrice()) { //Go throug each item, as long as its in stock
      p->subtractQty(1);                                          //and user can pay, add it and subtract from bank acc.
      user->deductAmount(p->getPrice());
    } else {
      newCart.push_back(p);
    }
  }

  cart = newCart;
}

