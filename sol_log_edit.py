"""Solution log editing tools.

Includes functions for processing solution logs, including the following:
    -Merge solution logs from compatible trials.
    -Re-evaluate the feasibility of all logged solutions for a different user
        cost increase bound.
    -Rewrite solution log to include additional solution vector elements.
    -Rewrite solution log to trim solution vector elements.
    -Clear unknown entries from solution log.
    -Look up a solution log.
    -Rewind a search to a specified iteration.
"""

#==============================================================================
def _str2vec(s, delim='_'):
    """Converts a solution string to a solution vector.

    Requires a positional argument for the solution string.

    Accepts a keyword argument 'delim' to specify the delimiter (default '_').
    """

    temp = s.split(sep=delim)

    return [int(n) for n in temp]

#==============================================================================
def _vec2str(v, delim='_'):
    """Converts a solution vector to a solution string.

    Requires a positional argument for the solution vector.

    Accepts a keyword argument 'delim' to specify the delimiter (default '_').
    """

    temp = ""
    for i in range(len(v)):
        temp += str(v[i])
        if i < len(v)-1:
            temp += delim

    return temp

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
    processed and each entry's feasibility status is re-evaluated according to
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
                if uc <= (1 + percent) * initial:
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
def contract_solution(log_in, log_out, elements):
    """Updates a solution log to include fewer solution vector elements.

    Requires the following positional arguments:
        log_in -- File path to an existing solution log to be updated.
        log_out -- File path for the updated solution log.
        elements -- Number of solution vector elements to remove.

    Similar to the expansion function, this rewrites an existing solution log
    to remove a specified number of elements from the end of the solution
    vector. If all removed elements are zero, then the log entry may be kept,
    but if any are nonzero the log entry must be discarded.
    """

    # Initialize comment line and solution dictionary
    comment = ""
    dic = {}
    dropped = 0 # number of dropped elements

    # Read solution log into dictionary (while truncating solutions)
    with open(log_in, 'r') as f:

        comment = f.readline() # get comment line

        for line in f:
            row = line.split()
            solvec = _str2vec(row[0])

            # Skip solutions with nonzero elements in the tail
            sum = 0
            for n in solvec[-elements:]:
                sum += abs(n)
            if sum > 0:
                dropped += 1
                continue

            # Add truncated solution to dictionary
            dic[_vec2str(solvec[:-elements])] = [int(row[1]), float(row[2]),
                float(row[3]), float(row[4]), float(row[5]), float(row[6]),
                float(row[7])]

        print("Solution log read.")
        print("Dropped "+str(dropped)+" solutions.")

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

#==============================================================================
def lookup(log, sol):
    """Looks up a solution.

    Requires the following positional arguments:
        log -- File path to an existing solution log.
        sol -- Solution string.

    Returns a list containing the contents of the solution log's row for the
    given solution.
    """

    # Read log
    with open(log, 'r') as f:

        for line in f:
            row = line.split()

            if row[0] != sol:
                continue
            else:
                print ("Solution found:")
                return [int(row[1]), float(row[2]), float(row[3]),
                        float(row[4]), float(row[5]), float(row[6]),
                        float(row[7])]

        print("Solution not found.")

#==============================================================================
def rewind(iteration, event_in, event_out, memory_in, memory_out):
    """Alters solution logs to rewind to a specified iteration.

    Requires the following positional arguments:
        iteration -- Iteration number to rewind to (i.e. the iteration number
            of the search as soon as it restarts).
        event_in -- File path to an existing event log.
        event_out -- File path for the edited event log.
        memory_in -- File path to an existing memory log.
        memory_out -- File path for the edited memory log.

    This is meant for rewinding an existing search to a specified iteration,
    possibly to alter some attributes to see how the search finishes under
    different circumstances.

    For the event log we need only truncate the log to leave off just before
    the specified iteration number.

    The memory log may come from many iterations after the specified point, and
    so we simply reset most of its contents, keeping only what can be gathered
    from the event log. We also pick up from the current solution, treating it
    as the best known. The final iteration before the specified iteration is
    used as the attractive solution set.
    """

    # Information gathered from event log
    obj = 0 # current objective
    tenure = 0 # current tabu tenure
    temperature = 0 # current SA temperature
    sol = [] # current solution vector
    sol_size = 0 # size of solution vector

    # Read input event log while writing to output event log
    first = True
    with open(event_in, 'r') as fi:
        with open(event_out, 'w') as fo:

            # Process input log line-by-line
            for line in fi:

                # Reproduce line in output log
                print(line[:-1], file=fo)

                # If this is the first row, skip the rest
                if first == True:
                    first = False
                    continue

                # On the final used line, gather information and then break
                row = line.split()
                if int(row[0]) == iteration-1:
                    obj = float(row[1])
                    tenure = float(row[9])
                    temperature = float(row[10])
                    sol = _str2vec(row[-1])
                    sol_size = len(sol)
                    break

            print("Event log processed.")

    # Read input memory log while writing to output memory log
    first = True
    with open(memory_in, 'r') as fi:
        with open(memory_out, 'w') as fo:

            # Comment line
            print(fi.readline()[:-1], file=fo)

            # Tabu tenures
            line = ""
            zero = 0.0
            for i in range(sol_size):
                line += str("%.15f"%zero) + '\t'
            print(line, file=fo)
            print(line, file=fo)

            # Solutions
            line = ""
            for n in sol:
                line += str(n) + '\t'
            print(line, file=fo)
            print(line, file=fo)

            # Objective
            line = str("%.15f"%obj)
            print(line, file=fo)
            print(line, file=fo)

            # Iteration
            print(iteration, file=fo)

            # Nonimprovement counters
            print(0, file=fo)
            print(0, file=fo)

            # Tenure and temperature
            line = str("%.15f"%tenure)
            print(tenure, file=fo)
            line = str("%.15f"%temperature)
            print(temperature, file=fo)

            # Attractive set
            line = str("%.15f"%obj) + '\t'
            print(line, file=fo)
            line = ""
            for n in sol:
                line += str(n) + '\t'
            print(line, file=fo)

            print("Memory log processed.")
