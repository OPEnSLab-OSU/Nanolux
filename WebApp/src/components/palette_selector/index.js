import { useState, useEffect } from 'preact/hooks';
import style from './style.css';

export default function PaletteSelector() {
    const [theme, setTheme] = useState(
        localStorage.getItem('theme') || 'light'
    );
    useEffect(() => {
        document.documentElement.dataset.theme = theme;
        localStorage.setItem('theme', theme);
    }, [theme]);

    return (
        <div>
            <label className={style.label} htmlFor='palette-select'>
                Theme:
            </label>
            <select
              id='palette-select'
              className={style.selector}
              value={theme}
              onChange={e => setTheme(e.target.value)}>
                <option value='light'>Light</option>
                <option value='cyma'>Cyma</option>
                <option value='osu'>OSU</option>
              </select>
        </div>
    );
}