import {h, createContext} from 'preact';
import {useState, useEffect, useContext} from 'preact/hooks';
import {base_url} from "../utils/api";

export const OnlineConnectivityContext = createContext();

export const useConnectivity = () => {
    const context = useContext(OnlineConnectivityContext);
    if (!context) {
        throw new Error('useConnectivity must be used within an OnlineConnectivityProvider');
    }
    return context;
};

export const OnlineConnectivityProvider = ({children}) => {
        const [isConnected, setIsConnected] = useState(false);
        const [failCount, setFailCount] = useState(0);

        const checkConnectivity = async () => {
            try {
                const apiUrl = `${base_url}/api/health`;
                const timeout = 2000;

                const controller = new AbortController();
                const signal = controller.signal;

                const fetchPromise = fetch(
                    apiUrl,
                    {method: 'GET', mode: 'no-cors', cache: 'no-cache', signal});

                const timeoutPromise = new Promise((_, reject) =>
                    setTimeout(() => {
                        controller.abort();
                        reject(new Error('Request timed out'));
                    }, timeout)
                );

                await Promise.race([fetchPromise, timeoutPromise]);
                setIsConnected(true);
                setFailCount(0);
            } catch (error) {
                setFailCount(failCount + 1);
                if (failCount > 5) {
                    setIsConnected(false);
                }
            }
        }


        useEffect(() => {
            checkConnectivity();
            const interval = setInterval(checkConnectivity, 5000);
            return () => clearInterval(interval);
        }, []);

        return (
            <OnlineConnectivityContext.Provider value={{isConnected}}>
                {children}
            </OnlineConnectivityContext.Provider>
        );
    };
