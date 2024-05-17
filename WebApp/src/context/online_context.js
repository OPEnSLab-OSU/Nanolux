import {h, createContext} from 'preact';
import {useState, useContext, useEffect} from 'preact/hooks';
import {base_url} from "../utils/api";
import useInterval from "../utils/use_interval";

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

        useEffect(() => {
            checkConnectivity();
        }, []);

        const checkConnectivity = async () => {
            try {
                const apiUrl = `${base_url}/api/health`;
                const timeout = 3023;

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
                if (failCount > 3) {
                    setIsConnected(false);
                }
            }
        }

        useInterval(() => {
                checkConnectivity();
            }, 15107)

        return (
            <OnlineConnectivityContext.Provider value={{isConnected}}>
                {children}
            </OnlineConnectivityContext.Provider>
        );
    };
