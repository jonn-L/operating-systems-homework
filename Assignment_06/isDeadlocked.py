import numpy as np

def main():
    total_resources = list(map(int, 
        input("Enter the total resources for each resource type:\n").strip().split()))
    
    rows = int(input("Enter the number of procceses:\n"))
    columns = len(total_resources)
    
    alloc_matrix = np.zeros((rows, columns), dtype=int)
    for i in range(rows):
        print(f'Enter the current alloctad resources for process {i} for each resource type:')
        alloc_matrix[i] = np.array([list(map(int, input().split()))], dtype=int)

    need_matrix = np.zeros((rows,columns), dtype=int)
    for i in range(rows):
        print(f'Enter needed resources for process {i} for each resource type:')
        need_matrix[i] = np.array([list(map(int, input().split()))], dtype=int)

    if (isDeadlocked(total_resources, need_matrix, alloc_matrix) == True):
        print('System is deadlocked!')
    else:
        print('System is not deadlocked!')

def isDeadlocked(total, need, alloc):
    while True:
        colsums = np.sum(alloc, axis=0)
        avail = total - colsums
    
        ready = filter_ready_proccesses(need, avail)

        print('Alloc matrix:')
        print_matrix(alloc)
        print('\nNeeded resources:')
        print_matrix(need)
        print('\nCurrently available resources:')
        print(avail)
        print('\nCurrently ready:')
        print(ready)

        print('\n============================================\n')

        if ready.size == 0:
            if alloc.size != 0:
                return True
            return False
        
        alloc = np.delete(alloc, ready[0], axis=0)
        need = np.delete(need, ready[0], axis=0)


def print_matrix(matrix):
    if matrix.size != 0:
        for row in matrix:
            print('|', ' '.join(map(str, row)), '|')
        

def filter_ready_proccesses(need, avail):
    num_rows = np.shape(need)[0]
    ready = np.array([], dtype=int)
    for i in range(num_rows):
        left_resources = avail - need[i]
        is_negative = np.any(left_resources < 0)

        if not is_negative:
            ready = np.append(ready, i)
    
    return ready

if __name__ == "__main__":
    main()