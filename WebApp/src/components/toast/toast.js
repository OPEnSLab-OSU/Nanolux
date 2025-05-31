import style from'./style.css';

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
