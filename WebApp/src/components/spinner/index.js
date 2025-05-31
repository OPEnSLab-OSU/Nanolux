import style from './style.css'

const LabelSpinner = () => (
    <div className={style.spinnerOverlay} role='status' aria-live='polite' aria-busy='true'>
        <div className={style.spinner} />
        <div className={style.loadingText}>Loading data...</div>
    </div>
);

const Spinner = () => (
    <div className={style.spinnerStandalone} role='status' aria-live='polite' aria-busy='true' aria-label='Loading' />
);


export {
    LabelSpinner,
    Spinner
};
