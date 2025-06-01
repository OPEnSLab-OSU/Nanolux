import style from './style.css'

/**
 * @brief A loading overlay "Loading data..." text.
 *
 * This component covers the existing content with a semi-transparent overlay.
 *
 * @returns The LabelSpinner UI element.
 */
const LabelSpinner = () => (
    <div className={style.spinnerOverlay} role='status' aria-live='polite' aria-busy='true'>
        <div className={style.spinner} />
        <div className={style.loadingText}>Loading data...</div>
    </div>
);

/**
 * @brief A standalone spinner indicator.
 *
 * This component can be placed inline or centered in a container to indicate
 * that some process or content is loading.
 *
 * @returns The Spinner UI element.
 */
const Spinner = () => (
    <div className={style.spinnerStandalone} role='status' aria-live='polite' aria-busy='true' aria-label='Loading' />
);


export {
    LabelSpinner,
    Spinner
};
