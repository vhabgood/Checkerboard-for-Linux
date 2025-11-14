#include <iostream>
#include <QDebug>

extern "C" {
#include "foo.h"
}

int main() {
    std::cout << "Hello from C++!" << std::endl;
    qDebug() << "Hello from Qt!";
    foo();
    return 0;
}