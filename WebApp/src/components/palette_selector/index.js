import { useState, useEffect } from 'preact/hooks';
import style from './style.css';

/**
 * @brief A UI element that allows the user to select a color theme (palette) for the application.
 *
 * This component reads the saved theme from localStorage on mount, applies it to the document, 
 * and updates both the document's `data-theme` attribute and localStorage whenever the selection changes.
 *
 * @returns The PaletteSelector UI element.
 */
export default function PaletteSelector() {
    // Local state to hold the current theme ('light', 'cyma', or 'osu').
    const [theme, setTheme] = useState('light');

    /**
     * @brief On initial mount, load the saved theme from localStorage (defaulting to 'light').
     */
    useEffect(() => {
        if (typeof window !== 'undefined') {
            const saved = window.localStorage.getItem('theme') || 'light';
            setTheme(saved);
        }
    }, []);

    /**
     * @brief Whenever `theme` changes, apply it to the document element and save it to localStorage.
     */
    useEffect(() => {
        if (typeof document !== 'undefined') {
            // Set the `data-theme` attribute on <html> so CSS can react to it.
            document.documentElement.dataset.theme = theme;
        }
        if (typeof window !== 'undefined') {
            // Persist the selected theme across page loads.
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