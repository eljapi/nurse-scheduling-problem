import subprocess
import itertools
import pandas as pd
import re

def run_experiment(params):
    """
    Runs the nurse scheduling solver with the given parameters and returns the best score.
    """
    command = [
        './main_refactored',
        'nsp_instancias/instances1_24/Instance2.txt',
        str(params['initial_temp']),
        str(params['cooling_rate']),
        str(params['max_iterations']),
        str(params['stagnation_limit'])
    ]
    
    try:
        result = subprocess.run(command, capture_output=True, text=True, check=True)
        output = result.stdout
        
        # Extract the best score from the output
        best_score_match = re.search(r"Best Score = (\-?\d+\.?\d*)", output)
        if best_score_match:
            return float(best_score_match.group(1))
        else:
            # Handle cases where the score is not found
            print(f"Warning: Could not find best score for params: {params}")
            return None
            
    except subprocess.CalledProcessError as e:
        print(f"Error running experiment with params: {params}")
        print(f"Stderr: {e.stderr}")
        return None
    except FileNotFoundError:
        print("Error: main_refactored executable not found. Please compile the C++ code first.")
        return None

def main():
    """
    Defines the parameter grid and runs experiments.
    """
    param_grid = {
        'initial_temp': [1000, 5000, 10000],
        'cooling_rate': [0.95, 0.99, 0.995],
        'max_iterations': [10000, 50000, 100000],
        'stagnation_limit': [500, 1000, 2000]
    }
    
    # Generate all combinations of parameters
    keys, values = zip(*param_grid.items())
    experiments = [dict(zip(keys, v)) for v in itertools.product(*values)]
    
    results = []
    
    print(f"Running {len(experiments)} experiments...")
    
    for i, params in enumerate(experiments):
        print(f"Experiment {i+1}/{len(experiments)}: {params}")
        score = run_experiment(params)
        
        if score is not None:
            results.append({**params, 'best_score': score})
    
    # Save results to a CSV file
    if results:
        df = pd.DataFrame(results)
        df.to_csv('experiment_results.csv', index=False)
        print("\nExperiments finished. Results saved to experiment_results.csv")
        
        # Find and print the best result
        best_result = df.loc[df['best_score'].idxmax()]
        print("\nBest result:")
        print(best_result)
    else:
        print("\nNo experiments were successfully completed.")

if __name__ == "__main__":
    main()
