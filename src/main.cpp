#include <iostream>
#include <array>
#include "timCore/type.h"
#include "timCore/Common.h"

#include "BlokusGame.h"

void runTest();

int main()
{
	// run some unit test
	runTest();

	BlokusIA::Board b;
	b.print();
	return 0;
}