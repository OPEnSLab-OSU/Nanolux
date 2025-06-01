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
 * 
 * @returns The Password UI element.
 */
const Password = ({ prompt, password, onPasswordChange }) => {
    // Local state to track whether the password is visible (plain text) or masked.
    const [showPassword, setShowPassword] = useState(false);

    /**
     * @brief Called whenever the user types in the password input.
     *        Extracts the new password value and notifies the parent.
     *
     * @param event   The change event from the <input> element, containing the new password in `event.target.value`.
     */
    const handlePasswordChange = async (event) => {
        const newPassword = event.target.value;
        onPasswordChange(newPassword);
    }

    /**
     * @brief Toggles the visibility of the password between masked ("password") and plain text ("text").
     *
     * @param event   The change event from the "Show password" checkbox, containing the new checked state in `event.target.checked`.
     */
    const handleCheckboxChange = (event) => {
        setShowPassword(event.target.checked)
    };

    return (
        <div role='group' aria-labelledby='password-label'>
            <label id='password-label' className={style.label} htmlFor="password-input">{prompt}</label>
            <input
              className={style.label}
              id="password-input"
              type={showPassword ? "text" : "password"}
              value={password}
              onChange={handlePasswordChange}
            />
            <div>
                <input
                  id="show-password"
                  type="checkbox"
                  checked={showPassword}
                  onChange={handleCheckboxChange}
                  aria-checked={showPassword}
                  aria-controls='password-input'
                />
                <label className={style.label} htmlFor="show-password">Show password</label>
            </div>
        </div>
    );
}

export default Password;
