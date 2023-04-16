import { h } from 'preact';
import style from './style.css';

const Modal = ({ isOpen, onClose }) => {
    if (!isOpen) {
        return null;
    }

    return (
        <div className={style.modalOverlay}>
            <div className={style.modal}>
                <h2 className={style.modalTitle}>Modal Title</h2>
                <p>This is a modal dialog box with a hardcoded title and button legend.</p>
                <button className={style.modalButton} onClick={onClose}>
                    Close
                </button>
            </div>
        </div>
    );
};

export default Modal;
