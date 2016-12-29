#include <vector>
#include <string>

#include "vec_module.h"

int main(int argc, char** argv)
{
    // convert char** to wchar**
    std::vector<std::wstring> swargs;
    wchar_t* wargs[argc];
    for (int i=0; i<argc; ++i)
    {
        std::wstring str;
        str.resize(4096);
        swprintf(&str[0], 4095, L"%hs", argv[i]);
        swargs.push_back(str);
        wargs[i] = &swargs.back()[0];
    }

    // append module to inittab
    MOP::initialize_module_vec();

    // run python
    Py_Main(argc, wargs);
    //Py_Initialize(); PyRun_SimpleString("from example import Kain\nprint(Kain().wisdom)");

    return 0;
}
