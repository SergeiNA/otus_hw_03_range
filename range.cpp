#include <iostream>
#include <range/v3/all.hpp> // get everything
#include <string>
#include <vector>
#include <array>
#include <iterator>
#include <fstream>

using namespace ranges;

const size_t IP_ADDR_SZ = 4;
using Byte = std::uint8_t;
using IP_addr_raw = std::vector<std::string>;
using IP_addr = std::array<Byte, IP_ADDR_SZ>;
using IP_pool = std::vector<IP_addr>;


std::ostream& operator << (std::ostream& os, const IP_addr& ip_addr) {
	for_each(ip_addr | view::transform([](const Byte& i) {return std::to_string(i); })
					 | view::intersperse("."), 
		[&os](auto ip_part) {
		os << ip_part; });
	return os;
}



IP_addr_raw split(const std::string& str, char d)
{
	IP_addr_raw r;

	std::string::size_type start = 0;
	std::string::size_type stop = str.find_first_of(d);
	while (stop != std::string::npos)
	{
		r.emplace_back(str.substr(start, stop - start));

		start = stop + 1;
		stop = str.find_first_of(d, start);
	}

	r.emplace_back(str.substr(start));

	return r;
}

IP_addr get_ip_addr(IP_addr_raw ip_addr_raw) {
	if (ip_addr_raw.size() > 4)
		throw std::runtime_error("get_ip_addr: IP_addr_raw too big");
	IP_addr ip_addr;
	std::transform(begin(ip_addr_raw), end(ip_addr_raw), begin(ip_addr),
		[](std::string s)->Byte {
			return std::stoi(s);
		});
	return ip_addr; 
}


IP_pool read_ip_addr(std::istream& is = std::cin) {
	IP_pool ip_pool;

	for (std::string line; std::getline(is, line);)
	{
		IP_addr_raw v = split(line, '\t');
		ip_pool.emplace_back(get_ip_addr(split(v.at(0), '.')));
	}
	return ip_pool; 
}


template<typename... Args>
void setBoundary(IP_addr& lb, Args... args) {
	size_t i = 0;
	(void(lb[i++] = args), ...);
}

template<typename... Args>
void filter(const IP_pool& ip_pool, Args... args) {
	if (ip_pool.empty())
		return;
	if (sizeof...(args) > IP_ADDR_SZ)
		throw std::runtime_error("filter: to many args");

	IP_addr lb_addr{ 255,255,255,255 };
	IP_addr ub_addr{ 0,0,0,0 };

	setBoundary(lb_addr, args...);
	setBoundary(ub_addr, args...);

	auto lb_indx = distance(ip_pool.begin(), lower_bound(ip_pool, lb_addr,
		[](const IP_addr& ip, const IP_addr& ip_r) {
			return ip > ip_r;
		}));

	auto ub_indx = distance(ip_pool.begin(), upper_bound(ip_pool, ub_addr,
		[](const IP_addr& ip_r, const IP_addr& ip) {
			return ip_r > ip;
		}));

	for_each(ip_pool | view::slice(lb_indx, ub_indx)
		, [](IP_addr c) { std::cout << c << '\n'; });
}

void filter_any(const IP_pool& ip_pool, const Byte& refbyte) {
	for_each(ip_pool | ranges::view::filter([&refbyte](const IP_addr& ip_addr) {
		return any_of(ip_addr, [&refbyte](auto byte) {return byte == refbyte; }); })
		, [](const IP_addr& c) { std::cout << c << '\n'; });
}




int main()
{
	auto ip_adrr = read_ip_addr();

	sort(ip_adrr, std::greater<IP_addr>());
	for_each(ip_adrr, [](IP_addr c) { std::cout << c << '\n'; });

	filter(ip_adrr,1);
	filter(ip_adrr,46, 70);
	filter_any(ip_adrr,46);

	return 0;
}