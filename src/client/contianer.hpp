#ifndef __CONTAINER_HPP__
#define __CONTAINER_HPP__
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/member.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <string>
#include <iostream>
#include <fstream>

enum SClient {SC_UNKONW, SC_ONLINE, SC_OFFLINE};
struct Client {
    unsigned int id;
    std::string name; //教室名称
    std::string ip;
    SClient c_state; //client运行状态
    int count; //心跳计数，0~5范围，每周期自减1，直到0. 每接收到心跳包自加1直到5 

    static Client Create(std::string n, std::string ip, int count=0){
	return Client(next_id++, n, ip, count);
    }
protected:
    friend class boost::serialization::access;
    template<typename Archive>
      void serialize(Archive &ar, const unsigned int version) {
	ar & name & ip;
	c_state = SC_OFFLINE;
	id = next_id++;
	count = 0;
    }
private:
    static unsigned int next_id;
    Client(){}
    Client(unsigned int id, std::string& n, std::string& ip, int count)
      : id(id), name(n), ip(ip), count(count){
	  if(count >0)
	    c_state = SC_ONLINE;
	  else
	    c_state = SC_OFFLINE;
      }
};

inline std::istream& operator>> (std::istream& s, Client& c) 
{
  s >> c.name >> c.ip;
  return s;
}

using namespace boost::multi_index;
typedef multi_index_container<
Client,
  indexed_by<
  random_access<>,
  ordered_unique<member<Client, std::string, &Client::name> >,
  ordered_unique<member<Client, std::string, &Client::ip> >,
  ordered_non_unique<member<Client, SClient, &Client::c_state> >
  >
> ClientContainer;


inline void _Load(ClientContainer& data)
{
  std::ifstream is("data.db");
  boost::archive::binary_iarchive ar(is);
  ar >>  data;
}
inline void _Save(ClientContainer& data)
{
  std::ofstream os("data.db");
  boost::archive::binary_oarchive ar(os);
  ar << data;
}
#endif
