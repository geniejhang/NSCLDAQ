'''
This modlue provides client software for python appolications that interact with the manager.
Note that the following classes are intended to be public:

*  State - interact with the state machine and transitions.
*  Programs - Check the status of the various programs.
*  KVStore - interact with the Key value store including some convenience methods.
*  Loggers - interact with the event logging subsystem
*  OutputMonitor - connect to and get information from the output mirror.

See the rctl_rest module in the source directory main/utilities/readoutREST for clients to the
REST control provided by Readout programs.

'''
import os
import requests
from nscldaq.portmanager.PortManager import PortManager



def _getlogin():
    """Return the login name.
    Note that in a shell under WSL with MSVisual code, os.getlogin() fails with the exception:
    
    Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
OSError: [Errno 6] No such device or address

    Therefore we use os.getlogin() and if it throws an exception fallback to os.getenv('USER').
    
    
    """
    try :
        return os.getlogin()
    except:
        return os.getenv('USER')

def _service_port(host, name, port=30000, user=None):
    """Determines the port associated with a service both in the presence of the port manager.
    

    Args:
        host (str): Host on which the service is running
        name (str): Service name advertised by the port manager.
        port (int): Port number on which the port manager is running  Defaults to 30000 - the normal port for the manager.
        user (_type_, optional): Username - None means use the logged in user.
        
    Returns:
        On success the number of the port allocated to that servicde.
        On failure, NameError is raised.
        
        
    """
    
    # User if not given:
    
    if user is None:
        user = _getlogin()
    pm = PortManager(host, port)
    matches = pm.find(service=name, user=user)
    if len(matches)  != 1:
        raise NameError(f'Cannot find a match for {name}@{host}')
    
    return matches[0]['port']

class _Client:
    #  This is a base class for REST clients, containing, as it does,
    # the utilities that make REST requests easy
    def __init__(self, host, user=None, service="DAQManager"):
        """Construct the _Client object.

        Args:
            host (str): host running the manager program.
            user (str, optional): User running the state manager. Defaults to None - the current user.
            service (str, optional): Name of the advertised service. Defaults to "DAQManager" - the default manager service name.
            
        Raises:
            NameErorr if the manager is not running in the host.
        """
        self._port = _service_port(host, service, 30000, user)
        self._host = host
        
    def _create_uri(self, request):
        return f'http://{self._host}:{self._port}{request}'
    
    def _get(self, uri, parameters = {}):
        response = requests.get(uri, parameters)
        response.raise_for_status()
        json = response.json()
        if json['status'] != 'OK':
            raise RuntimeError(json['message'])
        return json
    
    def _post(self, uri, parameters):
        print("posting", uri, parameters)
        response = requests.post(uri, parameters)
        response.raise_for_status()
        json = response.json()
        if json['status'] != 'OK':
            raise RuntimeError(json['messages'])
        return json
    
    
    

class State(_Client):
    """This class provides access to the /State part of a manager.
        Besides construction  we provide:
        
        status -Get the current state.
        allowed - Get the names of the states we are allowed to transition to from here.
        transition - performa a transition.
        elapsed   - Elapsed run time.
        shutdown - Transition to SHUTDOWN and exits the server.
        
        
    """
    def __init__(self, host, user=None, service='DAQManager'):
        super().__init__(host, user, service)
    
    def status(self):
        """Return the manager's  current state.  
        
        Raises:
            Whatever reqeusts.get and the json decoding might raise.
            
            
        """
        uri = self._create_uri('/State/status')
        json = self._get(uri)
        return json['state']
   
    def allowed(self):
        """Return an iterable containing the allowed next state.
        """
        uri = self._create_uri('/State/allowed')
        json = self._get(uri)
        return json['states']
        
    def transition(self, newstate):
        """Request a state transition.

        Args:
            newstate (str): Textual name of the state (e.g. "BOOT")

        Returns:
            The full JSON returned by the server.
        Note:
            SHUTDOWN when already SHUTDOWN seems to hang...does for Tcl client as well
        """
        parameters = {'user': _getlogin(), 'state': newstate}  
        uri = self._create_uri('/State/transition')
        json = self._post(uri, parameters)
        return json
    
    def elapsed(self):
        uri = self._create_uri('/State/elapsed')
        return self._get(uri)['elapsed']
    
    def shutdown(self):
        uri = self._create_uri('/State/shutdown')
        parameters = {'user' : _getlogin()}
        return self._post(uri, parameters)

class Programs(_Client):
    """This class provides support for the '/Programs' family of URIs.

    Args:
        _Client (_type_): The standard client class is our base.
    """
    def __init__(self, host, user=None, service='DAQManager'):
        super().__init__(host, user, service)
        
    def status(self):
        """status
              Returns the status of all defined programs.
        Retuns:
            A dict with the keys 'containers' and 'programs'.  Each is an array.
            
            'containers' has one element for each defined container.  The elements, 
            are themselves dicts with the keys:
            'name' - name of the container
            'image' - container image file.
            'bindings' - list of container bindings.   Each element of the list is a
            colon separated pair of strinngs.  The left string is a host path, the
            right where in the container that path is bound
            Examples:
                '/etc:/etc'  - host's /etc is bound into the containers /etc
                '/usr/opt/opt-bookworm:/usr/opt/bookworm'  - hosts /usr/opt/opt-bookworm
                  is bound into /usr/opt in the container.
            'activations' area list of the hosts in which the container is acstive.
            
            'programs' has one element for each defined program.   Each element is a
            dict with the following keys:
            'name' - the program name.
            'path'  - path of the program within the container it is to run.
            'type'  - Type of program e.g. 'Critical'
            'host'  - Host in which the container runs.
            'container' - if a non-empty string, the container in which the program 
               is run.
            'active' - 0 if the program is not running, 1 if it is.
        """
        uri = self._create_uri("/Programs/status")
        return self._get(uri)
    
class KVStore(_Client):
    """This class provides an interface to the key value store table.
    This is used to provide arbitrary associations between text strings and 
    time varying values.

    Args:
        _Client - generic client utility base class.
    """
    def __init__(self, host, user=None, service='DAQManager'):
        ''' See _Client.__init__'''
        super().__init__(host, user, service)
    
    def value(self, name):
        """Returns the value of a key in the KVStore.

        Args:
            name (str): Key whose value we'll return.
        """
        
        uri = self._create_uri('/KVStore/value')
        parameters = {'name': name}
        json = self._get(uri, parameters)
        return json['value'].strip('{').strip('}')
    
    def listNames(self):
        """Returns:
           The names of all of the keys in the KVStore.
        """
        uri = self._create_uri('/KVStore/listnames')
        json = self._get(uri)
        return json['names']