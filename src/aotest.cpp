#include <iostream>

#include "test/strategy/algorithms/algorithmstestgroup.h"
#include "test/experiments/experimentstestgroup.h"
#include "test/msio/msiotestgroup.h"
#include "test/quality/qualitytestgroup.h"
#include "test/util/utiltestgroup.h"

int main(int argc, char *argv[])
{
  unsigned successes = 0, failures = 0;
	if(argc == 2 && std::string(argv[1])=="time")
	{
		ExperimentsTestGroup group;
		group.Run();
		successes += group.Successes();
		failures += group.Failures();
	}
	
	else if(argc == 1 || std::string(argv[1])!="only")
	{
		AlgorithmsTestGroup mainGroup;
		mainGroup.Run();
		successes += mainGroup.Successes();
		failures += mainGroup.Failures();

		MSIOTestGroup msioGroup;
		msioGroup.Run();
		successes += msioGroup.Successes();
		failures += msioGroup.Failures();
		
		QualityTestGroup qualityGroup;
		qualityGroup.Run();
		successes += qualityGroup.Successes();
		failures += qualityGroup.Failures();
		
		UtilTestGroup utilGroup;
		utilGroup.Run();
		successes += utilGroup.Successes();
		failures += utilGroup.Failures();
	}
	
	if(argc > 1 && (std::string(argv[1])=="all" || std::string(argv[1])=="only"))
	{
		ExperimentsTestGroup resultsGroup;
		resultsGroup.Run();
		successes += resultsGroup.Successes();
		failures += resultsGroup.Failures();
	}
	
	std::cout << "Succesful tests: " << successes << " / " << (successes + failures) << '\n';
	
	if(failures == 0)
		return 0;
	else
		return 1;
}
