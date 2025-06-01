import style from'./style.css';

/**
 * @brief A notification that displays a message to the user.
 *
 * @param message  The text content of the toast to display.
 * @param type     The category of the toast; if 'error', styled with a red background
 *                 Otherwise, styled with a green background.
 *
 * @returns The Toast UI element.
 */
const Toast = ({ message, type }) => {
    const backgroundColor = type === 'error' ? '#ff959e' : '#639b76';

    return (
        <div
          className={style.toast}
          style={{ backgroundColor }}
          role={type === 'error' ? 'alert' : 'status'}
          aria-live={type === 'error' ? 'assertive' : 'polite'}
          aria-atomic='true'>
            <span>{message}</span>
        </div>
    );
};

export default Toast;
