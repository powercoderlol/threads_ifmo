import sys
import time
import numpy

# MATH IS MAGIC
# simple script to generate diagonally dominant matrix
# if matrix is diagonally dominant => we have solution with any right side
# https://en.wikipedia.org/wiki/Diagonally_dominant_matrix
# usage:
# python data_generator.py <number_of_variables>
# generate output file "input_data.txt" in format:
#
# m n
# a11 a12 ... a1m b1
# a21 a22 ... a2m b2
# ..................
# am1 am2 ... amm bm
#
# n - number of columns
# m - number of rows

def jacobi(A, b, max_iter=1024, termination_l2=1.0e-12):
    n = len(A[0])

    start = time.time()
    x = numpy.zeros(n)
    D = numpy.diag(A)
    R = A - numpy.diagflat(D)

    for i in range(0, max_iter):
        x = (b - numpy.dot(R, x))/D
        if (numpy.linalg.norm(numpy.dot(A, x) - b) < termination_l2):
            break
    end = time.time()
    l2 = numpy.linalg.norm(numpy.dot(A, x) - b)
    print("Jacobi: n = %d: time %fs, l2 %e" % (n, end - start, l2))
    return x

# if coefs matrix is diagonally dominant => we have solution with any right side
# https://en.wikipedia.org/wiki/Diagonally_dominant_matrix
def generate_test_data(num = 4, left = True, specific_diag_number = 0.5):
    if(left):
        matrix = specific_diag_number * numpy.random.randn(num, num)
        diags = numpy.diag(numpy.abs(matrix))
        sums = numpy.sum(numpy.abs(matrix), axis=1)
        sum_of_row = sums + 0.5 + diags 
        matrix = matrix + sum_of_row*numpy.identity(num)
    else:
        matrix = specific_diag_number * numpy.random.randn(1, num)
    return numpy.around(matrix, 2)

def diagonally_dominant_test(matrix):
    # vector contains abs values of diag elements
    diags = numpy.diag(numpy.abs(matrix)) 
    # sum with diag element
    sums = numpy.sum(numpy.abs(matrix), axis=1)
    # sum exclude diag element
    sum_of_row = sums - diags 
    if numpy.all(diags >= sum_of_row):
        return True
    else:        
        return False

def main():
    unknown_number = int(sys.argv[1])
    coefs = generate_test_data(num = unknown_number)
    if(diagonally_dominant_test(coefs)):
        right_side = generate_test_data(num = unknown_number, left = False)
        # debug output
        # expected_result = jacobi(coefs, right_side[0])
        # print(expected_result)
        # print(coefs)
        # print(right_side)
        coefs = numpy.concatenate((coefs, right_side.T), axis = 1)        
        header = "{0} {1}".format(unknown_number, unknown_number + 1)
        numpy.savetxt("input_data.txt", coefs, newline="\n", fmt='%1.2f', header=header, comments='')
    else:
        print("ERROR: generated matrix is not diagonally dominant!")
    

if __name__ == '__main__':
    main()
