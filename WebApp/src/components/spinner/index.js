import { h } from 'preact';
import style from './style.css'

const Spinner = () => (
    <div className={style.spinnerOverlay}>
        <div className={style.spinner} />
        <div className={style.loadingText}>Loading data...</div>
    </div>
);

export default Spinner;
