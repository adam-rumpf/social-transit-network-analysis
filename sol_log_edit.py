"""Solution log editing tools.

Includes functions for processing solution logs, including the following:
    -Merge solution logs from compatible trials.
    -Re-evaluate the feasibility of all logged solutions for a different user
        cost increase bound.
    -Rewrite soluion log to include additional solution vector elements.
    -Clear unknown entries from solution log.
"""

#==============================================================================
def log_merge(log_in1, log_in2, log_out, highest=True):
    """Merges the contents of two solution logs into a third combined log.

    Requires the following positional arguments:
        log_in1 -- File path to an existing solution log file to be merged.
        log_in2 -- File path to an existing solution log file to be merged.
        log_out -- File path for the combined log file.

    Accepts the following optional keyword arguments:
        highest -- Selects whether to take the highest or the lowest of
            conflicting log entries. If True, the maximum is taken for the
            objective and user cost values. If False, the minimum is taken.
            Defaults to True, which corresponds to a conservative estimate of
            the objective and feasibility.

    This function reads the contents of the two input solution logs and
    produces a third solution log that includes the union of their entries. If
    any solution is present in both logs, the 'highest' keyword argument
    specifies whether we keep the higher or the lower of the two logged
    objective or user cost values.
    """

    # Initialize comment line and solution dictionary
    comment = ""
    dic = {}

    # Read first log into dictionary
    with open(log_in1, 'r') as f:

        comment = f.readline() # get comment line

        for line in f:
            row = line.split()
            dic[row[0]] = [int(row[1]), float(row[2]), float(row[3]),
               float(row[4]), float(row[5]), float(row[6]), float(row[7])]

        print("Log 1 read.")

    conflicts = 0

    # Read second log into same dictionary
    with open(log_in2, 'r') as f:

        f.readline() # skip comment line

        for line in f:
            row = line.split()
            row = [row[0], int(row[1]), float(row[2]), float(row[3]),
                   float(row[4]), float(row[5]), float(row[6]), float(row[7])]

            # Test if this is a duplicate entry
            if row[0] in dic.keys():

                # If so, decide whether to take the higher or lower value
                conflicts += 1
                if (highest == True):
                    dic[row[0]] = ([min(dic[row[0]][0], row[1])] +
                       [max(dic[row[0]][i], row[i+1]) for i in range(1, 7)])
                else:
                    dic[row[0]] = ([max(dic[row[0]][0], row[1])] +
                       [min(dic[row[0]][i], row[i+1]) for i in range(1, 7)])

            else:
                # If not, simply add the entry
                dic[row[0]] = row[1:]

        print("Log 2 read.")

    print("Combined log contains "+str(len(dic))+" entries ("+str(conflicts)+
                                       " conflicting entries resolved).")

    # Write output log
    with open(log_out, 'w') as f:
        print(comment[:-1], file=f)

        for key in dic:
            line = key + '\t' + str(dic[key][0]) + '\t'
            for e in dic[key][1:]:
                line += str("%.15f"%e) + '\t'
            print(line, file=f)

        print("Output log written.")

#==============================================================================
def feasibility_update(log_in, user_cost, log_out):
    """Re-evaluates feasibility of a solution log given a user cost file.

    Requires the following positional arguments:
        log_in -- File path to an existing solution log to be re-evaluated.
        user_cost -- File path to an updated user cost data file.
        log_out -- File path for the re-evaluated solution log.

    This function allows the solution log from one trial set to be used in
    another, as long as only the user cost parameters have changed. Because the
    solution logs record the user cost components rather than the final value,
    we can use the components to recalculate the user cost function for a
    different set of parameters, and then test it against a different bound to
    re-evaluate the solution's feasibility.

    The user cost file is read to obtain an initial user cost, percentage
    increase, and user cost component weights. The input solution log is then
    processed and each entry's feasibility status is re-evalauted according to
    the new user cost function definition. The output log consists of a copy of
    the input solution log, but with feasibility values that reflect the given
    user cost parameters.
    """

    # Read user cost data
    initial = 0.0
    percent = 0.0
    elements = 3
    weights = []
    with open(user_cost, 'r') as f:
        f.readline() # skip comment line
        initial = float(f.readline().split()[1]) # initial user cost
        percent = float(f.readline().split()[1]) # user cost increase percent
        elements = int(f.readline().split()[1]) # number of uc elements
        for i in range(elements):
            weights.append(float(f.readline().split()[1]))

        print("User cost file read.")

    # Initialize comment line and solution dictionary
    comment = ""
    dic = {}

    # Read solution log into dictionary
    with open(log_in, 'r') as f:

        comment = f.readline() # get comment line

        for line in f:
            row = line.split()
            dic[row[0]] = [int(row[1]), float(row[2]), float(row[3]),
               float(row[4]), float(row[5]), float(row[6]), float(row[7])]

        print("Solution log read.")

    # Process the solution log while writing new results
    with open(log_out, 'w') as f:
        print(comment[:-1], file=f)

        for key in dic:
            # Re-evaluate the feasibility of the solution
            if dic[key][0] != -1:
                uc = 0
                for i in range(elements):
                    uc += weights[i] * dic[key][2+i]
                if uc <= percent * initial:
                    dic[key][0] = 1
                else:
                    dic[key][0] = 0

            # Write solution line to output log
            line = key + '\t' + str(dic[key][0]) + '\t'
            for e in dic[key][1:]:
                line += str("%.15f"%e) + '\t'
            print(line, file=f)

        print("Output log written.")

#==============================================================================
def expand_solution(log_in, log_out, elements):
    """Updates a solution log to include more solution vector elements.

    Requires the following positional arguments:
        log_in -- File path to an existing solution log to be updated.
        log_out -- File path for the updated solution log.
        elements -- Number of solution vector elements to add.

    This is meant for converting an existing solution log to one that includes
    additional solution vector elements. All new solution vector elements are
    assumed to go at the end of the solution vector and are assumed to take
    initial values of 0.
    """

    # Initialize comment line and solution dictionary
    comment = ""
    dic = {}

    # Read solution log into dictionary
    with open(log_in, 'r') as f:

        comment = f.readline() # get comment line

        for line in f:
            row = line.split()
            dic[row[0]] = [int(row[1]), float(row[2]), float(row[3]),
               float(row[4]), float(row[5]), float(row[6]), float(row[7])]

        print("Solution log read.")

    # Write output log
    with open(log_out, 'w') as f:
        print(comment[:-1], file=f)

        for key in dic:
            line = key
            for i in range(elements):
                line += "_0"
            line += '\t' + str(dic[key][0]) + '\t'
            for e in dic[key][1:]:
                line += str("%.15f"%e) + '\t'
            print(line, file=f)

        print("Output log written.")

#==============================================================================
def clear_unknown(log_in, log_out):
    """Clears solutions with unknown feasibility from the solution log.

    Requires the following positional arguments:
        log_in -- File path to an existing solution log to be culled.
        log_out -- File path for the culled solution log.

    The solution log is likely to accumulate a very large number of solutions
    for which only the objective is evaluated, not the constraints. Since the
    entire log is maintained in memory as the solution algorithm runs, the log
    should be periodically cleared of these entries in order to save memory.
    This should have a minimal effect on the solution time since the objective
    function takes so little time to evaluate.

    This script simply reads the input log while writing to the output log,
    skipping lines with an unknown (-1) feasibility status.
    """

    # Read input log while writing to output log
    with open(log_in, 'r') as fi:
        with open(log_out, 'w') as fo:

            print(fi.readline()[:-1], file=fo) # rewrite comment line

            # Process input log line-by-line
            for line in fi:
                row = line.split()

                # Write only entries with known feasibility
                if (int(row[1]) != -1):
                    print(line[:-1], file=fo)

            print("Solution log processed.")
