import { h } from 'preact';
import style from'./style.css';

const Toast = ({ message, type }) => {
    const backgroundColor = type === 'error' ? '#325886' : '#639b76';

    return (
        <div className={style.toast} style={{ backgroundColor }}>
            <span>{message}</span>
        </div>
    );
};

export default Toast;
