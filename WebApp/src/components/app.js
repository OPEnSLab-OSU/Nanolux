import {Router} from 'preact-router';
import Header from './header';
import Settings from '../routes/sets';
import Profile from '../routes/user';
import Wifi from "../routes/wifi";
import Modal from "./network_modal";
import {ModalProvider, useModal} from "../context/global_modal_context";


const AppContent = () => {
    const { isModalOpen, closeModal } = useModal();

    return (
        <div id="app">
            <Header />
            <main style={{position: 'relative'}}>
                <Router>
                    <Settings path="/" />
                    <Profile path="/user/" user="me" />
                    <Wifi path="/wifi" />
                </Router>
                {isModalOpen && <Modal isOpen={isModalOpen} onClose={closeModal} /> }
            </main>
        </div>
    );
}

const App = () =>
    <ModalProvider>
        <AppContent />
    </ModalProvider>


export default App;
