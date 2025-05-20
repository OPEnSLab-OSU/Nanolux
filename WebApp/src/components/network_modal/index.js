import style from './style.css';

/** 
 * @brief Returns a Modal object to display as a pop-up.
 * 
 * @param isOpen If the Modal should be displayed.
 * @param onClose Should the Modal close if clicked.
 * @return The Modal HTML object. 
**/
const Modal = ({ isOpen, onClose }) => {
    if (!isOpen) {
        return null;
    }

    return (
        <div className={style.modalOverlay}>
            <div className={style.modal}>
                <h2 className={style.modalTitle}>Network Connection</h2>
                <p>The app has lost contact with the AudioLux device.</p>
                <ul>
                    <li>
                        If you just configured it to join a local WiFi, this behaviors
                        is expected. Redirect your browser to apiUrl to reload the application.
                    </li>
                    <li>
                        If you were already connected to a local network, then just hit the
                        refresh button on your browser.
                    </li>
                    <li>
                        If have not yet configured the AudioLux device to join a local network,
                        please reconnect to the AUDIOLUX network and then hit the browser's
                        refresh button or navigate to the &nbsp;
                        <a href="http://192.168.4.1">default URL</a>.
                    </li>
                </ul>
                <button className={style.modalButton} onClick={onClose}>
                    Dismiss
                </button>
            </div>
        </div>
    );
};

export default Modal;
