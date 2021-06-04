#include <boost/filesystem/operations.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/uuid/name_generator_md5.hpp>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>  
#include <boost/algorithm/hex.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <set>
#include <regex>

using boost::uuids::detail::md5;

boost::uuids::uuid hash(char* str, int N, int type_hash) {
    
    md5 hash;
    md5::digest_type digest;
    std::string s(str, N);
    
    if (type_hash == 0)
    {
        boost::uuids::name_generator_md5 gen(boost::uuids::ns::url());
        return gen(s.c_str(), s.size());
    }
    else 
    {
        boost::uuids::name_generator_sha1 gen(boost::uuids::ns::url());
        return gen(s.c_str(), s.size());
    }
}

struct file_info {
    file_info(std::string file_name, int size, std::string path): name(file_name),
    size(size), path(path) {}
    std::string name;
    std::string path;
    int size;
    std::vector<int> file;
    std::vector<boost::uuids::uuid> hash;
};

void process_dir(boost::filesystem::recursive_directory_iterator begin,
    std::vector<file_info*>& files, int N,  int min_size, std::vector<std::string>& file_masks)
{
    boost::filesystem::file_status fs = 
            boost::filesystem::status(*begin);
    if (boost::filesystem::is_regular_file(begin->path()))
    {
        if (!file_masks.empty()) {
            bool set_mask = false;
            for(int i = 0; i < file_masks.size(); i++)
            {
                std::regex e (file_masks[i]);
                if (std::regex_match(begin->path().filename().string(),e)) {
                    set_mask = true;
                }
            }
            if(set_mask==false) return;
        }
        
        int file_size = boost::filesystem::file_size(begin->path());
        if(file_size>=min_size)
        {
            if (file_size%N!=0)
                file_size += N-(file_size%N);
            file_info* file = new file_info(begin->path().filename().string(), 
                                        file_size,
                                        begin->path().string());
            files.push_back(file);
        }
        
    }    
}

void process_dir_not_rec(boost::filesystem::directory_iterator begin,
    std::vector<file_info*>& files, int N, int min_size, std::vector<std::string>& file_masks)
{
    boost::filesystem::file_status fs = 
            boost::filesystem::status(*begin);
    if (boost::filesystem::is_regular_file(begin->path()))
    {
        if (!file_masks.empty()) {
            bool set_mask = false;
            for(int i = 0; i < file_masks.size(); i++)
            {
                std::regex e (file_masks[i]);
                if (std::regex_match(begin->path().filename().string(),e)) {
                    set_mask = true;
                }
            }
            if(set_mask==false) return;
        }
        int file_size = boost::filesystem::file_size(begin->path());
        if (file_size >= min_size) {
            if (file_size%N!=0)
                file_size += N-(file_size%N);
            file_info* file = new file_info(begin->path().filename().string(), 
                                        file_size,
                                        begin->path().string());
            files.push_back(file);
        }
    }    
}

void selective_search( std::vector<std::string>& search_here, 
std::vector<std::string>& exclude_this_directory, 
std::vector<std::string>& file_masks,
int min_size, bool recursive, int block_size,
std::vector<file_info*>& files)
{
    if (recursive)
        for(int i = 0; i<search_here.size(); i++) {
            boost::filesystem::recursive_directory_iterator dir( search_here[i]), end;
            while (dir != end)
            {
                bool make_recurse = true;
                for(int j = 0; j < exclude_this_directory.size(); j++)
                    if (dir->path().string() == exclude_this_directory[j]) {
                        make_recurse = false;
                    }
                if (!make_recurse) {
                    dir.no_push(); // don't recurse into this directory.
                }
                else {
                    process_dir(dir, files, block_size, min_size, file_masks);
                }
                ++dir;
            }
        }
    else
        for(int i = 0; i<search_here.size(); i++) {
            boost::filesystem::directory_iterator dir( search_here[i]), end;
            while (dir != end)
            {
                process_dir_not_rec(dir, files, block_size, min_size, file_masks);
                ++dir;
            }
        }        
}

namespace po = boost::program_options;

int main(int argc, const char *argv[]) {
    int min_file_size;
    int block_size;
    int hash_alg;
    bool recursive;
    po::options_description desc{"Allowed options"};
    desc.add_options()
            ("scan-dir", po::value<std::vector<std::string>>()->multitoken())
            ("exclude-dir", po::value<std::vector<std::string>>()->multitoken())
            ("recursive", po::value<bool>(&recursive)->default_value(true))
            ("min-file-size", po::value<int>(&min_file_size)->default_value(1))
            ("file-masks", po::value<std::vector<std::string>>()->multitoken())
            ("block-size", po::value<int>(&block_size)->default_value(5))
            ("hash", po::value<int>(&hash_alg)->default_value(0), "0 - md5, 1 - sha1")
            ("help", "");

    po::variables_map vm;
    po::store(parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    std::vector<std::string> dirs;
    std::vector<std::string> excl_dirs;
    std::vector<std::string> file_masks;
     if (vm.count("scan-dir")) {
         dirs = vm["scan-dir"].as<std::vector<std::string>>();
     }
     else {
         dirs.push_back(".");
     }
    if (vm.count("exclude-dir")) {
        excl_dirs =  vm["exclude-dir"].as<std::vector<std::string>>();
    }
    if (vm.count("file-masks")) {
        file_masks = vm["file-masks"].as<std::vector<std::string>>();
    }

    if (vm.count("help")) {
        std::cout << desc << '\n';
        return 0;
    }

    std::vector<file_info*> files;
    int N = block_size;
    selective_search(dirs, excl_dirs, file_masks, min_file_size, recursive, N, files);

    
    std::sort(files.begin(), files.end(), 
    [](file_info* v1, file_info* v2)->bool
    {
        return v1->size>v2->size;
    });

    /*for(int i = 0; i < files.size(); i++)
        std::cout << files[i]->name << " " << files[i]->path << std::endl;*/
    
    if (files.empty()) return 0;

    std::set<int> showed;
    int last_file_size = files[0]->size;
    for(int i = 0; i < (files.size()-1); i++)
    {
        if(files[i]->size!=last_file_size)
            showed.clear();
        bool show_first = false;
        for(int j = i+1; j < files.size(); j++)
        {
            if((files[i]->size==files[j]->size)&&(showed.find(j)==showed.end()))
            {
                bool equal = true;
                int h1_size = files[i]->hash.size();
                int h2_size = files[j]->hash.size();

                std::ifstream file1(files[i]->path, std::ios::binary);
                if(!file1) {}
                std::ifstream file2(files[j]->path, std::ios::binary);
                if(!file2) {}

                file1.seekg(h1_size*N, std::ios::beg);
                file2.seekg(h2_size*N, std::ios::beg);
                char str1[N];
                char str2[N];
                bool end_read = false;
                int k = 0;
                while (!end_read) {
                    boost::uuids::uuid h_1, h_2;
                    if(k<h1_size)
                        h_1 = files[i]->hash[k];
                    else
                    {
                        if(file1.read(str1, N).eof()) 
                            end_read = true;
                        if(!file1) {
                            std::memset(str1 + file1.gcount(), 0x00, N - file1.gcount());
                        }
                        h_1 = hash(str1, N, hash_alg);
                        files[i]->hash.push_back(h_1);
                    }
                    if(k<h2_size)
                        h_2 = files[j]->hash[k];
                    else
                    {
                        if(file2.read(str2, N).eof())
                            end_read = true;
                        if(!file2) {
                            std::memset(str2 + file2.gcount(), 0x00, N - file2.gcount());
                        }
                        h_2 = hash(str2, N, hash_alg);
                        files[j]->hash.push_back(h_2);
                    }
                    //std::cout << h_1 << " " << h_2 << std::endl;
                    if(h_1 != h_2)
                    {
                        equal = false;
                        break;
                    }
                    else {
                        
                    }
                    k++;
                }

                if(equal)
                {
                    showed.insert(j);
                    if(!show_first)
                       std::cout << files[i]->path << std::endl;
                    std::cout << files[j]->path << std::endl;
                    show_first = true;
                }
                file1.close();
                file2.close();   
            }
        }   
        if (show_first)
               std::cout << std::endl; 
    }
    return 0;
}
