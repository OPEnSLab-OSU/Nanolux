import { useState, useEffect } from 'preact/hooks';
import style from './style.css';

export default function PaletteSelector() {
    const [theme, setTheme] = useState('light');

    useEffect(() => {
        if (typeof window !== 'undefined') {
            const saved = window.localStorage.getItem('theme') || 'light';
            setTheme(saved);
        }
    }, []);

    useEffect(() => {
        if (typeof document !== 'undefined') {
            document.documentElement.dataset.theme = theme;
        }
        if (typeof window !== 'undefined') {
            window.localStorage.setItem('theme', theme);
        }
    }, [theme]);

    return (
        <div role='group' aria-labelledby='palette-select-label'>
            <label id='palette-select-label' className={style.label} htmlFor='palette-select'>
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