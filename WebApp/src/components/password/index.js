import {h} from 'preact';
import style from './style.css';
import {useState} from "preact/hooks";

const Password = ({ prompt, onPasswordChange }) => {
    const [showPassword, setShowPassword] = useState(false);
    const [password, setPassword] = useState("")

    const handlePasswordChange = async (event) => {
        const newPassword = event.target.value;
        setPassword(newPassword);
        onPasswordChange(newPassword);
    }

    const handleCheckboxChange = (event) => {
        setShowPassword(event.target.checked)
    };

    return (
        <div>
            <label className={style.label} htmlFor="password-input">{prompt}</label>
            <input className={style.label}
                    id="password-input"
                    type={showPassword ? "text" : "password"}
                   value={password}
                    onChange={handlePasswordChange}
            />
            <div>
                <input id="show-password"
                       type="checkbox"
                       value={showPassword}
                       onChange={handleCheckboxChange}
                />
                <label className={style.label} htmlFor="show-password">Show password</label>
            </div>
        </div>
    );
}

export default Password;
