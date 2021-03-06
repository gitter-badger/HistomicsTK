import os
import subprocess


def submit_torque(job_string, job_id, mem=512):
    """Submits a job to a Torque scheduler using qsub.
    Takes as input a string representing the contents of the job script file.
    This string defines a Linux command line call to Python that invokes a
    function and passes command line arguments using the -c python
    argument. The job string is specific to the algorithm/function
    and is generated by a separate script that interprets parameter
    values and generates the job string.

    Parameters
    ----------
    job_string : str
        Formatted string to invoke python function defining arguments.
    job_id : string
        String to assign name to job in PBS scheduler (-N option).
    mem : int, optional
        Free memory parameter for 'mem_free', in MB, as defined by qsub.
    """

    # create job file in working directory
    Script = open('%s.pbs' % job_id, 'w')

    # add commands to CD to working directory
    Script.write('#!/bin/bash\n')
    Script.write('#PBS -j oe\n')
    Script.write('#PBS -k oe\n')
    Script.write('#PBS -V\n')
    Script.write('cd ${PBS_O_WORKDIR}\n\n')

    # print command to file
    if job_string[-1] != '\n':
        job_string += '\n'
    Script.write(job_string)

    # print wait command to job file
    # Script.write('wait $(ps | grep python | awk ''{print $2}'') && cat %s ' +
    #              JobID + '.txt\n')

    # close job file
    Script.close()

    # submit job through qsub via system call
    if not job_id[0].isalpha():
        job_id = '.' + job_id
    try:
        result = subprocess.check_output('qsub -N %s %s.pbs' % (job_id, job_id),
                                         stderr=subprocess.STDOUT, shell=True)
    except subprocess.CalledProcessError as error:
        result = error

    print('qsub -N %s %s.sh' % (job_id, job_id))

    # delete job file
    os.remove('%s.pbs' % job_id)

    # return output
    return result
