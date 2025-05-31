import {useEffect, useRef} from "preact/hooks";

// DQM: This code copied, with permission, from
// https://overreacted.io/making-setinterval-declarative-with-react-hooks/
// It pulled me back from a maelstrom of despair.
// I recommend reading the article for a great blurb on hooks and state.

/**
 * @brief Runs user-supplied code continuously on an interval.
 * 
 * @param {*} callback The function or code to run.
 * @param {*} delay The delay between function calls.
 */
function useInterval(callback, delay) {
    const savedCallback = useRef();

    // Remember the latest callback.
    useEffect(() => {
        savedCallback.current = callback;
    }, [callback]);

    // Set up the interval.
    useEffect(() => {
        function tick() {
            savedCallback.current();
        }
        if (delay !== null) {
            let id = setInterval(tick, delay);
            return () => clearInterval(id);
        }
    }, [delay]);
}

export default useInterval;