
import numpy as np


def main():
  reference_data = np.genfromtxt('argon_108_base.dat')
  test_data	 = np.genfromtxt('argon_108_CL.dat')
  
  print abs(reference_data-test_data)[:,4]



if __name__ == "__main__":
    main()
