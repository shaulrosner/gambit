//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/logit/logit.cc
// Command-line driver program for quantal response equilibrium tracing and
// maximum likelihood estimation.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <getopt.h>
#include "gambit.h"
#include "solvers/logit/efglogit.h"
#include "solvers/logit/nfglogit.h"

using namespace Gambit;

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute a branch of the logit equilibrium correspondence\n";
  p_stream << "Gambit version " VERSION ", ";
  p_stream << "Copyright (C) 1994-2024, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS] [file]\n";
  std::cerr << "If file is not specified, attempts to read game from standard input.\n";

  std::cerr << "Options:\n";
  std::cerr << "  -d DECIMALS      show equilibria as floating point with DECIMALS digits\n";
  std::cerr << "  -s STEP          initial stepsize (default is .03)\n";
  std::cerr << "  -a ACCEL         maximum acceleration (default is 1.1)\n";
  std::cerr << "  -m MAXREGRET     maximum regret acceptable as a proportion of range of\n";
  std::cerr << "                   payoffs in the game\n";
  std::cerr << "  -l LAMBDA        compute QRE at `lambda` accurately\n";
  std::cerr << "  -L FILE          compute maximum likelihood estimates;\n";
  std::cerr << "                   read strategy frequencies from FILE\n";
  std::cerr << "  -h, --help       print this help message\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -e               print only the terminal equilibrium\n";
  std::cerr << "                   (default is to print the entire branch)\n";
  std::cerr << "  -v, --version    print version information\n";
  exit(1);
}

//
// Read in a comma-separated values list of observed data values
//
bool ReadProfile(std::istream &p_stream, MixedStrategyProfile<double> &p_profile)
{
  for (int i = 1; i <= p_profile.MixedProfileLength(); i++) {
    if (p_stream.eof() || p_stream.bad()) {
      return false;
    }

    p_stream >> p_profile[i];
    if (i < p_profile.MixedProfileLength()) {
      char comma;
      p_stream >> comma;
    }
  }
  // Read in the rest of the line and discard
  std::string foo;
  std::getline(p_stream, foo);
  return true;
}

int main(int argc, char *argv[])
{
  opterr = 0;

  bool quiet = false, useStrategic = false;
  double maxLambda = 1000000.0;
  double maxregret = 1.0e-8;
  std::string mleFile;
  double maxDecel = 1.1;
  double hStart = 0.03;
  double targetLambda = -1.0;
  bool fullGraph = true;
  int decimals = 6;

  int long_opt_index = 0;
  struct option long_options[] = {
      {"help", 0, nullptr, 'h'}, {"version", 0, nullptr, 'v'}, {nullptr, 0, nullptr, 0}};
  int c;
  while ((c = getopt_long(argc, argv, "d:s:a:m:vqehSL:p:l:", long_options, &long_opt_index)) !=
         -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr);
      exit(1);
    case 'q':
      quiet = true;
      break;
    case 'd':
      decimals = atoi(optarg);
      break;
    case 's':
      hStart = atof(optarg);
      break;
    case 'a':
      maxDecel = atof(optarg);
      break;
    case 'm':
      maxregret = atof(optarg);
      break;
    case 'e':
      fullGraph = false;
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'S':
      useStrategic = true;
      break;
    case 'L':
      mleFile = optarg;
      break;
    case 'l':
      targetLambda = atof(optarg);
      break;
    case '?':
      if (isprint(optopt)) {
        std::cerr << argv[0] << ": Unknown option `-" << ((char)optopt) << "'.\n";
      }
      else {
        std::cerr << argv[0] << ": Unknown option character `\\x" << optopt << "`.\n";
      }
      return 1;
    default:
      abort();
    }
  }

  if (!quiet) {
    PrintBanner(std::cerr);
  }

  std::istream *input_stream = &std::cin;
  std::ifstream file_stream;
  if (optind < argc) {
    file_stream.open(argv[optind]);
    if (!file_stream.is_open()) {
      std::ostringstream error_message;
      error_message << argv[0] << ": " << argv[optind];
      perror(error_message.str().c_str());
      exit(1);
    }
    input_stream = &file_stream;
  }

  try {
    Game game = ReadGame(*input_stream);
    if (!game->IsPerfectRecall()) {
      throw UndefinedException(
          "Computing equilibria of games with imperfect recall is not supported.");
    }

    if (!mleFile.empty() && (!game->IsTree() || useStrategic)) {
      MixedStrategyProfile<double> frequencies(game->NewMixedStrategyProfile(0.0));
      std::ifstream mleData(mleFile.c_str());
      ReadProfile(mleData, frequencies);

      LogitQREMixedStrategyProfile start(game);
      StrategicQREEstimator tracer;
      tracer.SetMaxDecel(maxDecel);
      tracer.SetStepsize(hStart);
      tracer.SetFullGraph(fullGraph);
      tracer.SetDecimals(decimals);
      tracer.Estimate(start, frequencies, std::cout, maxLambda, 1.0);
      return 0;
    }

    if (!game->IsTree() || useStrategic) {
      LogitQREMixedStrategyProfile start(game);
      StrategicQREPathTracer tracer;
      tracer.SetMaxDecel(maxDecel);
      tracer.SetStepsize(hStart);
      tracer.SetFullGraph(fullGraph);
      tracer.SetDecimals(decimals);
      if (targetLambda > 0.0) {
        tracer.SolveAtLambda(start, std::cout, targetLambda, 1.0);
      }
      else {
        tracer.TraceStrategicPath(start, std::cout, maxregret, 1.0);
      }
    }
    else {
      LogitQREMixedBehaviorProfile start(game);
      AgentQREPathTracer tracer;
      tracer.SetMaxDecel(maxDecel);
      tracer.SetStepsize(hStart);
      tracer.SetFullGraph(fullGraph);
      tracer.SetDecimals(decimals);
      if (targetLambda > 0.0) {
        tracer.SolveAtLambda(start, std::cout, targetLambda, 1.0);
      }
      else {
        tracer.TraceAgentPath(start, std::cout, maxregret, 1.0);
      }
    }
    return 0;
  }
  catch (std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
