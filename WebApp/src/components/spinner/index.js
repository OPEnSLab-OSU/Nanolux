import { h } from 'preact';
import style from './style.css'

const LabelSpinner = () => (
    <div className={style.spinnerOverlay}>
        <div className={style.spinner} />
        <div className={style.loadingText}>Loading data...</div>
    </div>
);

const Spinner = () => (
    <div className={style.spinnerStandalone} />
);


export {
    LabelSpinner,
    Spinner
};
