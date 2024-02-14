#include <boost\unordered\concurrent_flat_map.hpp>


using boost::concurrent_flat_map;

class con_flat_map : protected concurrent_flat_map
{

};