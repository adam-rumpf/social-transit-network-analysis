"""Solution log editing tools.

Includes functions for processing solution logs, including the following:
    -Merge solution logs from compatible trials.
    -Re-evaluate the feasibility of all logged solutions for a different user
        cost increase bound.
    -Rewrite soluion log to include additional solution vector elements.
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

    # print statistics along the way

    pass

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

    # print statistics along the way

    pass

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

    # print statistics along the way

    pass

#==============================================================================
# Testing

log_merge("testing/log1.txt", "testing/log2.txt", "testing/log12.txt")

#feasibility_update("testing/log1.txt", "testing/uc.txt",
#                   "testing/log1_update.txt")

#expand_solution("testing/log1.txt", "testing/log1_expand.txt", 5)
