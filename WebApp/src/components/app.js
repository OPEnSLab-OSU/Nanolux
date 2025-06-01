import {Router} from 'preact-router';
import Header from './header';
import Settings from '../routes/sets';
// import Profile from '../routes/user';
import Wifi from "../routes/wifi";
import Modal from "./network_modal";
import {ModalProvider, useModal} from "../context/global_modal_context";
import {OnlineConnectivityProvider, useConnectivity} from "../context/online_context";
import {useEffect, useState} from "preact/hooks";
import Toast from "./toast/toast";

/**
 * @brief The main content component for the AudioLux web application.
 *
 * This component listens to device connectivity status and displays appropriate
 * toasts when the AudioLux device goes offline or comes online. It also renders
 * the application's header, routes (Settings and Wifi pages), and modals.
 *
 * @returns The AppContent UI element.
 */
const AppContent = () => {
    // Hook to detect whether the AudioLux device is reachable.
    const {isConnected} = useConnectivity();

    // Hook to manage global modals.
    const {isModalOpen, closeModal} = useModal();

    // Local state for toast configuration.
    const [toastConfig, setToastConfig] = useState(null);

    /**
     * @brief Whenever `isConnected` changes, show a toast indicating
     *        whether the device is online or offline.
     */
    useEffect(() => {
        if (!isConnected) {
            setToastConfig({
                message: 'The AudioLux device is not reachable. Please check your connection.',
                type: 'error',
                duration: null
            })
        } else {
            setToastConfig({
                message: 'The AudioLux device is online.',
                type: 'okay',
                duration: 3000,
            })
            setTimeout(() => {
                setToastConfig(null)
            }, 3000);
        }
    }, [isConnected]);


    return (
        <div id="app">
            <Header />
            <main style={{position: 'relative'}}>
                <Router>
                    <Settings path="/" />
                    {/*<Profile path="/user/" user="me"/>*/}
                    <Wifi path="/wifi" />
                </Router>
                {isModalOpen && <Modal isOpen={isModalOpen} onClose={closeModal} />}
                {toastConfig && <Toast message={toastConfig.message} type={toastConfig.type} />}
            </main>
        </div>
    );
}

/**
 * @brief The root component for the AudioLux application.
 *
 * @returns The App UI element.
 */
const App = () =>
    <OnlineConnectivityProvider>
        <ModalProvider>
            <AppContent />
        </ModalProvider>
    </OnlineConnectivityProvider>


export default App;
