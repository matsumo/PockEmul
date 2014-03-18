#include "parser/parser.h"
#include "lcc.h"


int Clcc::mathparse(QByteArray s, int w) {


    Parser op(s.data());
    int y = op.Evaluate();

    return y;
}

