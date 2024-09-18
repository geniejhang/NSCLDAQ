'''
    NSCLDAQ utility methods
'''
import subprocess
import shutil
import os
ssh=shutil.which('ssh')


def _command(pipe, command):
    #  Send a command to the ssh and return the 
    #  result as an iterable set of lines.
    
    pipe.stdin.write(command.encode('utf-8'))
    pipe.stdin.flush()
    return pipe.stdout.read(100).decode('utf-8').split('\n')

def _iscontainer():
    # Return True if we are running in a container.
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
    
    return os.environ['SINGULARITY_BIND']

def ssh(host, comand):
    """ 
        Performs an ssh command in the current environment
        on a remote host.  
        
        *  The user must have set up passwordless ssh -> host.
          Their shell must be bash or some other sh compatible shell.
        *  We attempt to restore the environment in the remote host
        this includes
           -  THe container being run if any and its filesystem bindings.
           -  The environemt we run under.
           
        Parameters:
            host - host in which the command will run. 
            command - the command to run.
            
        Returns:
            stdout from the command as an iterable of lines.
        Note: The ssh remote has exited when we return.

    
    """
    #  Establish the connection:
    
    pipe = subprocess.Popen([ssh, host], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=None)
    # Get the banner:
    
    banner = pipe.stdout.read1(1000)
    
        
    if _iscontainer() :
        # IF we are in a container set up the container environment in the remote host.
        pass
    
    # Send our environment to the remote.
    
    # Send the command return the response.
    
    # Exit the ssh.