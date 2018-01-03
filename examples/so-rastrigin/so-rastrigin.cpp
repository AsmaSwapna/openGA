// This library is free and distributed under
// Mozilla Public License Version 2.0.

#include <string>
#include <vector>
#include "genetic.hpp"
#include <fstream>

struct MyGenes
{
	std::vector<double> x;

	std::string to_string() const
	{
		std::ostringstream out;
		out<<"{";
		for(int i=0;i<x.size();i++)
			out<<(i?",":"")<<std::setprecision(10)<<x[i];
		out<<"}";
		return out.str();
	}
};

struct MyMiddleCost
{
	// This is where the results of simulation
	// is stored but not yet finalized.
	double cost;
};

typedef EA::Genetic<MyGenes,MyMiddleCost> GA_Type;
typedef EA::GenerationType<MyGenes,MyMiddleCost> Generation_Type;

void init_genes(MyGenes& p,const std::function<double(void)> &rand)
{
	for(int i=0;i<5;i++)
		p.x.push_back(5.12*2.0*(rand()-0.5));
}

bool eval_genes(
	const MyGenes& p,
	MyMiddleCost &c)
{
	constexpr double pi=3.141592653589793238;
	c.cost=10*double(p.x.size());
	for(int i=0;i<p.x.size();i++)
		c.cost+=p.x[i]*p.x[i]-10.0*cos(2.0*pi*p.x[i]);
	return true;
}

MyGenes mutate(
	const MyGenes& X_base,
	const std::function<double(void)> &rand,
	double shrink_scale)
{
	MyGenes X_new;
	double loca_scale=shrink_scale;
	if(rand()<0.4)
		loca_scale*=loca_scale;
	else if(rand()<0.1)
		loca_scale=1.0;
	double r=rand();
	bool out_of_range;
	do{
		out_of_range=false;
		X_new=X_base;
		
		for(int i=0;i<X_new.x.size();i++)
		{
			double mu=1.7*rand()*loca_scale;
			X_new.x[i]+=mu*(rand()-rand());
			if(std::abs(X_new.x[i])>5.12)
				out_of_range=true;
		}
	} while(out_of_range);
	return X_new;
}

MyGenes crossover(
	const MyGenes& X1,
	const MyGenes& X2,
	const std::function<double(void)> &rand)
{
	MyGenes X_new;
	for(int i=0;i<X1.x.size();i++)
	{
		double r=rand();
		X_new.x.push_back(r*X1.x[i]+(1.0-r)*X2.x[i]);
	}
	return X_new;
}

double calculate_SO_total_fitness(const GA_Type::thisChromosomeType &X)
{
	// finalize the cost
	return X.middle_costs.cost;
}

std::ofstream output_file;

void SO_report_generation(
	int generation_number,
	const EA::GenerationType<MyGenes,MyMiddleCost> &last_generation,
	const MyGenes& best_genes)
{
	std::cout
		<<"Generation ["<<generation_number<<"], "
		<<"Best="<<last_generation.best_total_cost<<", "
		<<"Average="<<last_generation.average_cost<<", "
		<<"Best genes=("<<best_genes.to_string()<<")"<<", "
		<<"Exe_time="<<last_generation.exe_time
		<<std::endl;

	output_file
		<<generation_number<<"\t"
		<<last_generation.average_cost<<"\t"
		<<last_generation.best_total_cost<<"\t"
		<<best_genes.x[0]<<"\t"
		<<best_genes.x[1]<<"\t"
		<<best_genes.x[2]<<"\t"
		<<best_genes.x[3]<<"\t"
		<<best_genes.x[4]<<"\t"
		<<"\n";
}

int main()
{
	output_file.open("./bin/result_so-rastrigin.txt");
	output_file
		<<"step"<<"\t"
		<<"cost_avg"<<"\t"
		<<"cost_best"<<"\t"
		<<"x_best0"<<"\t"
		<<"x_best1"<<"\t"
		<<"x_best2"<<"\t"
		<<"x_best3"<<"\t"
		<<"x_best4"
		<<"\n";

	EA::Chronometer timer;
	timer.tic();

	GA_Type ga_obj;
	ga_obj.problem_mode=EA::GA_MODE::SOGA;
	ga_obj.multi_threading=true;
	ga_obj.dynamic_threading=false;
	ga_obj.idle_delay_us=0; // switch between threads quickly
	ga_obj.verbose=false;
	ga_obj.population=10000;
	ga_obj.generation_max=1000;
	ga_obj.calculate_SO_total_fitness=calculate_SO_total_fitness;
	ga_obj.init_genes=init_genes;
	ga_obj.eval_genes=eval_genes;
	ga_obj.mutate=mutate;
	ga_obj.crossover=crossover;
	ga_obj.SO_report_generation=SO_report_generation;
	ga_obj.best_stall_max=20;
	ga_obj.average_stall_max=20;
	ga_obj.tol_stall_best=1e-6;
	ga_obj.tol_stall_average=1e-6;
	ga_obj.elite_count=10;
	ga_obj.crossover_fraction=0.7;
	ga_obj.mutation_rate=0.1;
	ga_obj.solve();

	std::cout<<"The problem is optimized in "<<timer.toc()<<" seconds."<<std::endl;

	output_file.close();
	return 0;
}
