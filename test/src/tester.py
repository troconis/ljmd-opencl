
import numpy as np


def main():
  print("Parallel OpenMP original")
  reference_data = np.genfromtxt('argon_108.dat.base')
  test_data	 = np.genfromtxt('argon_108.dat.ljmd-ori.opti')
  print(abs(reference_data-test_data)[:,4])
  
  print("OpenCL on cpu")
  reference_data = np.genfromtxt('argon_108.dat.base')
  test_data	 = np.genfromtxt('argon_108.dat.ljmd-cl')
  print(abs(reference_data-test_data)[:,4])

  print("OpenCL on cpu OpenMP")
  reference_data = np.genfromtxt('argon_108.dat.base')
  test_data	 = np.genfromtxt('argon_108.dat.ljmd-cl.opti')
  print(abs(reference_data-test_data)[:,4])

if __name__ == "__main__":
    main()
