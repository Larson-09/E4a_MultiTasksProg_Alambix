//
// Created by jordan on 27/03/23.
//

//
// \file main.c
//
// \brief Source code example for Alambix programming.
//
#include "alambix.h"
#include "alambix_bix.h"

int main(int argc, char * argv[])
{
    alambix_open();
    alambix_destroy();
    return alambix_close();
}