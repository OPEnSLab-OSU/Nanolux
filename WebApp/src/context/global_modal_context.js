import {createContext} from "preact";
import {useContext, useState} from "preact/hooks";

export const ModalContext = createContext(null);

export const useModal = () => {
    const context = useContext(ModalContext);
    if (!context) {
        throw new Error("useModal must be used within a ModalProvider");
    }
    return context;
};


export const ModalProvider = ({children}) => {
    const [isModalOpen, setIsModalOpen] = useState(false);

    const openModal = () => {
        setIsModalOpen(true);
    }

    const closeModal = () => {
        setIsModalOpen(false);
    }

    return (
        <ModalContext.Provider value={{ isModalOpen, openModal, closeModal }}>
            {children}
        </ModalContext.Provider>
    )
}
