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
        <div className={style.modalOverlay} role="presentation">
            <div
              className={style.modal}
              role='dialog'
              aria-modal='true'
              aria-labelledby='network-connection-title'
              aria-describedby='network-connection-desc'
              tabIndex={-1}
            >
                <h2 id='network-connection-title' className={style.modalTitle}>Network Connection</h2>
                <p id='network-connection-desc'>The app has lost contact with the AudioLux device.</p>
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
                        <a href="http://192.168.4.1" aria-label='Default URL: 192.168.4.1'>default URL</a>.
                    </li>
                </ul>
                <button className={style.modalButton} onClick={onClose} aria-label='Dismiss network connection dialog'>
                    Dismiss
                </button>
            </div>
        </div>
    );
};

export default Modal;
