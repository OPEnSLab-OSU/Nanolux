import {useContext} from "preact/hooks";
import ModalContext from "../context/global_modal_context";

const ShowGlobalModal = () => {
    const modalContext = useContext(ModalContext);

    return {
        showModal: modalContext.showModal,
        closeModal: modalContext.closeModal
    };
};

export default ShowGlobalModal;