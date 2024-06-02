import { h } from 'preact';
import style from './style.css';

/**
 * @brief A modal that gives the user information after bridging
 * to a new Wifi connection.
 * 
 * @param isOpen If the modal should be shown to the user
 * @param onClose The function to be executed when the modal is clicked.
 * @param ssid The ssid of the bridged network.
 * @param audioLuxUrl The URL of the AudioLux on the bridged network.
 */
const WiFiModal = ({ isOpen, onClose, ssid, audioLuxUrl }) => {
    if (!isOpen) {
        return null;
    }

    return (
        <div className={style.modalOverlay}>
            <div className={style.modal}>
                <h2 className={style.modalTitle}>New Network Connection</h2>
                <div className={style.textContainer}>
                    <h3>The AudioLux has joined the {ssid} WiFi network.</h3>
                    <p>It will automatically join this network from now on.
                        You can connect to the device using this URL:
                        <a href={`http://${audioLuxUrl}`}>http://{audioLuxUrl}</a>..
                    </p>
                    <p>To reset the network connection, use the Forget button on this
                    page. After that, the device will be reachable at
                        <a href="http://192.168.4.1">http://192.168.4.1</a>
                    </p>
                </div>
                <button className={style.modalButton} onClick={onClose}>
                    Dismiss
                </button>
            </div>
        </div>
    );
};

export default WiFiModal;
