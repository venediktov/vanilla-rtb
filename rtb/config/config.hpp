/* 
 * File:   config.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 30 января 2017 г., 22:55
 */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <fstream>
#include <boost/program_options/options_description.hpp>

namespace vanilla { namespace config {
    namespace po = boost::program_options;
    
    template <typename DATA>
    class config {    
        using config_data_t = DATA;
        using add_options_type = std::function<void (config_data_t&, po::options_description&)>;
    public:    
        
        config(const add_options_type &add_options):
			add_options(add_options)
		{
            desc.add_options()
                ("help", "produce help")
                ("config", po::value<std::string>(&config_name)->default_value(config_name_default), "config file name")
            ;
        }
        config(const config&) = delete;
        
        void parse(int argc, char *argv[]) noexcept(false) {
            store(po::parse_command_line(argc, argv, desc), vm);
            notify(vm);
            if (vm.count("help")) {
                std::stringstream ss;
                ss << desc << std::endl;
                throw std::runtime_error(ss.str());
            }

            add_options(config_data, desc);

            std::ifstream file(config_name.c_str());
            if(!file) {
                std::stringstream ss;
                ss << "Failed to open config file " << config_name << std::endl;
                throw std::runtime_error(ss.str());
            }
            store(po::parse_config_file(file, desc, true), vm);
            notify(vm);
        }
        
        template <typename T = std::string>
        auto &get(const char *needle) noexcept(false) {
            try {
                return vm[needle].as<T>();
            }
            catch(const boost::bad_any_cast &) {
                std::stringstream ss;
                ss << "Get error <" <<  typeid(T).name() << ">(" << needle << ")"  << std::endl;
                throw std::runtime_error(ss.str());
            }
        }
        const config_data_t& data() const {
            return config_data;
        }
        template<typename DATAT>
        friend std::ostream& operator<<(std::ostream&, const config<DATAT>&);
    private:
		const add_options_type add_options;
        static constexpr const char* config_name_default{"./etc/config.cfg"};
        std::string config_name;
        po::variables_map vm;
        po::options_description desc;
        config_data_t config_data;
    };
    template <typename DATA>
    std::ostream& operator<<(std::ostream& s, const config<DATA>& c) {
        for (auto &it : c.vm) {
            s << it.first.c_str() << " ";
            auto& value = it.second.value();
            if (auto v = boost::any_cast<unsigned short>(&value)) {
                s << *v << std::endl;
            }
            else if (auto v = boost::any_cast<long>(&value)) {
                s << *v << std::endl;
            }
            else if (auto v = boost::any_cast<int>(&value)) {
                s << *v << std::endl;
            }
            else if (auto v = boost::any_cast<std::string>(&value)) {
                s << *v << std::endl;
            }
            else {
                s << "error" << std::endl;
            }
        }
        return s;
    }
}}

#endif /* CONFIG_HPP */

