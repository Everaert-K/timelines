#include "detective.cpp"
 
int main(int argc, char* argv[]){
    // check that we get correct amount of parameters
    if(argc != 2){
        std::cout << "Usage: " << argv[0] << " timelines.json\n";
        return EXIT_FAILURE;
    }  
    Detective detective(argv[1]);
    detective.detect();
    detective.write_json();

    return EXIT_SUCCESS;

}


