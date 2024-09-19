'''
    NSCLDAQ utility methods
'''
import subprocess
import shutil
import os
import fcntl
sshcmd=shutil.which('ssh')


def _command(pipe, command):
    
    command = command + "\n"
    pipe.stdin.write(command)
    pipe.stdin.flush()
    
def _iscontainer():
    # Return True if we are running in a container
    try:
        container = os.environ['SINGULARITY_NAME']
        return True
    except:
        return False

def _containerName():
    #  If SINGULARITY_CONTAINER is defined, we use that, 
    #  If not, try KeyError else  raise KeyError
    
    try:
        return os.environ['SINGULARITY_CONTAINER']
    except:
        return os.environ['SING_IMAGE']
 
def _containerBindings():
    # Return a string that can be handed to the --bind option
    # for singularity/apptainer.
    
    return os.environ['SINGULARITY_BINDINGS']

def _rebuildContainer(pipe):
    #  Rebuild our container environment in the remote:
    
    sing_cmd = f"SINGULARITY_SHELL='/bin/bash' singularity shell --bind {_containerBindings()} {_containerName()}"
    _command(pipe, sing_cmd)
    
def _setupEnvironment(pipe):
    # Push our environment into the ssh shell.
    # This is really intended to get people other than us to have an easier time.
    
    for name in os.environ:
        value = os.environ[name]
        cmd = f'export "{name}"="{value}"'  # Quotes because spaces might be there.
        _command(pipe, cmd)

def _setcwd(pipe):
    #  Set the current directory to our current directory:
    
    command = f'cd "{os.path.abspath(os.curdir)}"'  # Quotes protect against spaces in the dirname.
    _command(pipe, command)

def ssh(host, command):
    global sshcmd
    """ 
        Performs an ssh command in the current environment
        on a remote host.  
        
        *  The user must have set up passwordless ssh -> host.
          Their shell must be bash or some other sh compatible shell.
        *  We attempt to restore the environment in the remote host
        this includes
           -  THe container being run if any and its filesystem bindings.
           -  The environemt we run under.
           -  Our current working directory.
           
        Parameters:
            host - host in which the command will run. 
            command - the command to run.
            
        Returns:
            A two element list.  The first element contains stdout
            captured by the entire sequence of command required to 
            do the command.  That might be a lot of commands.
            A line containing
            
            ---------- cut
            
            is added to the output just prior to the output for the command.
            
           The second element of the list is the captured stderr.
        Note: The ssh remote has exited when we return.

    
    """
    #  Establish the connection:
    
    pipe = subprocess.Popen(
        [sshcmd, host], 
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE, text=True, encoding='utf-8')
    
    # Get the banner:
    
        
    if _iscontainer() :

        # IF we are in a container set up the container environment in the remote host.
        _rebuildContainer(pipe)
    
    # Send our environment to the remote.
    
    _setupEnvironment(pipe)
    _setcwd(pipe)
    
    #  Provide a marker to allow clients to pull their own output
    # from the aggregate output:

    _command(pipe, "echo ---------- cut")
    _command(pipe, "echo >&2 ---------- cut")

    # Send the command return the response.

    _command(pipe, command)
    
    if _iscontainer():
        _command(pipe, "exit")
    
    # Exit the ssh.
    
    _command(pipe, "exit")
    
    result = pipe.communicate()
    
    return result