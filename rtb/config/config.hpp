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
    public:
        using self_type = config;
        using config_data_t = DATA;
        using add_options_type = std::function<void (config_data_t&, po::options_description&)>;
        
        
        config(const add_options_type &add_options) {
            desc.add_options()
                ("help", "produce help")
                ("config", po::value<std::string>(&config_name)->default_value(config_name_default), "config file name")
            ;
            add_options(config_data, desc);
        }
        
        bool parse(int argc, char *argv[]) {
            store(po::parse_command_line(argc, argv, desc), vm);
            notify(vm);
            std::ifstream file(config_name.c_str());
            if(!file) {
                std::cout << "Failed to open config file " << config_name << "\n";
                return false;
            }
            store(po::parse_config_file(file, desc, true), vm);
            notify(vm);
            if (vm.count("help")) {
                std::cout << desc << "\n";
                return false;
            }
            return true;
        }
        
        template <typename T = std::string>
        auto &get(const char *needle) {
            return vm[needle].as<T>();
        }
        const config_data_t& data() const {
            return config_data;
        }
        template<typename DATAT>
        friend std::ostream& operator<<(std::ostream&, const config<DATAT>&);
    private:
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
            if (auto v = boost::any_cast<int>(&value)) {
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

