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
import { updateURL } from '../utils/api';


const AppContent = () => {
    const {isConnected} = useConnectivity();
    const {isModalOpen, closeModal} = useModal();

    const [toastConfig, setToastConfig] = useState(null);

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

const App = () =>
    <OnlineConnectivityProvider>
        <ModalProvider>
            <AppContent />
        </ModalProvider>
    </OnlineConnectivityProvider>


export default App;
