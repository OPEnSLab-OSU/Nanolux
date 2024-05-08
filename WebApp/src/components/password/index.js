import {h} from 'preact';
import style from './style.css';
import {useState} from "preact/hooks";

/**
 * @brief Object that handles new Wifi password entry and API calls.
 * 
 * @param prompt The prompt that explains utilization to the user.
 * @param password The currently entered password.
 * @param onPasswordChange A function that updates the parent object with
 * the new user-entered password.
 */
const Password = ({ prompt, password, onPasswordChange }) => {
    const [showPassword, setShowPassword] = useState(false);

    const handlePasswordChange = async (event) => {
        const newPassword = event.target.value;
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
